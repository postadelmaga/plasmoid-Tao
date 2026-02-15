// Compilazione:
// g++ main.cpp -o tao_anim -std=c++20 -O3 -lSDL3 -pthread

#include <SDL3/SDL.h>
#include <vector>
#include <cmath>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <random>

// --- CONFIGURAZIONE ---
const int WIN_W = 1024;
const int WIN_H = 768;
const int MAX_PARTICLES = 3000; // Bilanciato per estetica

struct {
    float rotSpeed = 0.5f;      // Velocità rotazione Tao
    int   pCount   = 1200;      // Numero particelle
    float glowInt  = 0.8f;      // Intensità luce
    float flowSpd  = 1.0f;      // Velocità fisica particelle
} cfg;

// --- DATI ---
// Struttura stile C (Cartesiana invece che Polare)
struct Particle {
    float x, y;     // Posizione
    float vx, vy;   // Velocità (Vector)
    float life;     // Vita 0.0 - 1.0
    float decay;    // Velocità morte
    float size;     // Dimensione
};

struct Slider {
    SDL_FRect r; float* val; float min, max; SDL_Color col; bool drag;
};

// --- GLOBALS ---
std::vector<Particle> particles(MAX_PARTICLES);
SDL_Texture* texGlow = nullptr;
SDL_Texture* texTao  = nullptr;
int scW = WIN_W, scH = WIN_H;

// --- WORKER THREADS ---
class WorkerPool {
    std::vector<std::thread> th;
    std::condition_variable cv;
    std::mutex mtx;
    std::atomic<int> done{0};
    std::atomic<bool> work{false}, exit{false};
    int count;
    float cx, cy, rad;

public:
    WorkerPool() {
        count = std::max(2u, std::thread::hardware_concurrency());
        for(int i=0; i<count; ++i) th.emplace_back([this, i]{ loop(i); });
    }
    ~WorkerPool() {
        exit = true; cv.notify_all();
        for(auto& t : th) if(t.joinable()) t.join();
    }

    void update(float _cx, float _cy, float _rad) {
        cx = _cx; cy = _cy; rad = _rad;
        done = 0;
        { std::lock_guard<std::mutex> lk(mtx); work = true; }
        cv.notify_all();
        while(done.load(std::memory_order_acquire) < count) std::this_thread::yield();
        work = false;
    }

private:
    void loop(int idx) {
        // RNG Locale per thread (più veloce di rand() globale)
        std::mt19937 rng(std::hash<std::thread::id>{}(std::this_thread::get_id()) + idx);
        std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
        std::uniform_real_distribution<float> distM11(-1.0f, 1.0f);

        while(true) {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [this]{ return work.load() || exit.load(); });
            if(exit) return;
            lk.unlock();

            int total = cfg.pCount;
            int chunk = total / count;
            int start = idx * chunk;
            int end = (idx == count - 1) ? total : (idx + 1) * chunk;

            float _cx = cx, _cy = cy, _r = rad;
            float spd = cfg.flowSpd;

            for(int i=start; i<end; ++i) {
                Particle& p = particles[i];

                // Aggiornamento stile C (x += vx)
                p.x += p.vx * spd;
                p.y += p.vy * spd;
                p.life -= p.decay * spd;

                // Respawn
                if(p.life <= 0) {
                    p.life = 1.0f;
                    // Nascono in un anello attorno al Tao
                    float angle = dist01(rng) * M_PI * 2.0f;
                    // Distanza: Tra 1.1x e 1.6x il raggio del Tao
                    float dist = _r * (1.1f + dist01(rng) * 0.5f);

                    p.x = _cx + cosf(angle) * dist;
                    p.y = _cy + sinf(angle) * dist;

                    // Velocità: Drift casuale lento (come nel file C)
                    // Aggiungiamo una leggera spinta centrifuga per non farle collassare
                    p.vx = (cosf(angle) * 0.5f + distM11(rng) * 0.5f) * 0.5f;
                    p.vy = (sinf(angle) * 0.5f + distM11(rng) * 0.5f) * 0.5f;

                    p.decay = 0.002f + dist01(rng) * 0.005f;
                    p.size = 0.5f + dist01(rng) * 1.5f; // Dimensione variabile
                }
            }
            done.fetch_add(1, std::memory_order_release);
            while(work && !exit) std::this_thread::yield();
        }
    }
};

// --- GRAPHICS ---
SDL_Texture* MakeGlow(SDL_Renderer* ren, int s) {
    SDL_Surface* surf = SDL_CreateSurface(s, s, SDL_PIXELFORMAT_RGBA8888);
    SDL_LockSurface(surf);
    Uint32* p = (Uint32*)surf->pixels;
    float c = s/2.0f;
    for(int i=0; i<s*s; ++i) {
        float x = (i%s)-c, y = (i/s)-c;
        float d = sqrtf(x*x+y*y);
        // Glow più "core" e meno sfumato, simile ai pallini del C ma più bello
        float a = (d<c) ? powf(1.0f-d/c, 4.0f) : 0.0f;
        p[i] = SDL_MapRGBA(SDL_GetPixelFormatDetails(surf->format), nullptr, 255,255,255, (Uint8)(a*255));
    }
    SDL_UnlockSurface(surf);
    auto t = SDL_CreateTextureFromSurface(ren, surf);
    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_ADD);
    SDL_DestroySurface(surf);
    return t;
}

void BakeTao(SDL_Renderer* ren, int s) {
    if(texTao) SDL_DestroyTexture(texTao);
    texTao = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, s, s);
    SDL_SetTextureBlendMode(texTao, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(ren, texTao);
    SDL_SetRenderDrawColor(ren, 0,0,0,0); SDL_RenderClear(ren);

    auto Circle = [&](float cx, float cy, float r, float a1, float a2, SDL_Color col) {
        std::vector<SDL_Vertex> v; std::vector<int> ix;
        v.push_back({{cx,cy}, {col.r/255.f,col.g/255.f,col.b/255.f,col.a/255.f}, {0,0}});
        int n=80; // Più segmenti per rotondità perfetta
        for(int i=0; i<=n; ++i) {
            float a = a1 + (a2-a1)*((float)i/n);
            v.push_back({{cx+cosf(a)*r, cy+sinf(a)*r}, {col.r/255.f,col.g/255.f,col.b/255.f,col.a/255.f}, {0,0}});
            if(i>0) { ix.push_back(0); ix.push_back(i); ix.push_back(i+1); }
        }
        SDL_RenderGeometry(ren, nullptr, v.data(), v.size(), ix.data(), ix.size());
    };

    float c=s/2.0f, r=s/2.0f-2;
    Circle(c,c,r, 0, M_PI*2, {235,235,235,255});
    Circle(c,c,r, -M_PI/2, M_PI/2, {10,10,10,255});
    Circle(c,c-r/2,r/2, 0, M_PI*2, {10,10,10,255});
    Circle(c,c+r/2,r/2, 0, M_PI*2, {235,235,235,255});
    Circle(c,c-r/2,r/6, 0, M_PI*2, {255,255,255,255});
    Circle(c,c+r/2,r/6, 0, M_PI*2, {0,0,0,255});

    // Bordo sottile grigio per definizione
    SDL_SetRenderTarget(ren, nullptr);
}

// --- MAIN ---
int main(int, char**) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* win = SDL_CreateWindow("Tao Final v5 (C-Style Particles)", WIN_W, WIN_H, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* ren = SDL_CreateRenderer(win, nullptr);
    SDL_SetRenderVSync(ren, 1);
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

    texGlow = MakeGlow(ren, 32); // Texture piccola per le particelle
    BakeTao(ren, 1024); // Texture enorme per il Tao (dettaglio massimo)
    WorkerPool pool;

    std::vector<Slider> sliders = {
        {{20,20,150,15}, &cfg.rotSpeed, 0.0f, 2.0f, {255,100,100,255}, 0},
        {{20,55,150,15}, (float*)&cfg.pCount, 100.f, (float)MAX_PARTICLES, {100,255,100,255}, 0},
        {{20,90,150,15}, &cfg.glowInt, 0.1f, 2.0f, {100,100,255,255}, 0},
        {{20,125,150,15}, &cfg.flowSpd, 0.0f, 3.0f, {255,255,100,255}, 0}
    };

    bool run = true;
    SDL_Event ev;
    float time = 0, rot = 0;

    while(run) {
        while(SDL_PollEvent(&ev)) {
            if(ev.type == SDL_EVENT_QUIT) run = false;
            if(ev.type == SDL_EVENT_WINDOW_RESIZED) SDL_GetRenderOutputSize(ren, &scW, &scH);

            float mx = ev.button.x, my = ev.button.y;
            if(ev.type == SDL_EVENT_MOUSE_MOTION) mx = ev.motion.x, my = ev.motion.y;

            if(ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
                for(auto& s : sliders) if(mx>=s.r.x && mx<=s.r.x+s.r.w && my>=s.r.y && my<=s.r.y+s.r.h) s.drag=true;
                if(ev.type == SDL_EVENT_MOUSE_BUTTON_UP)
                    for(auto& s : sliders) s.drag=false;
                    if(ev.type == SDL_EVENT_MOUSE_MOTION)
                        for(auto& s : sliders) if(s.drag) {
                            float v = s.min + std::clamp((mx-s.r.x)/s.r.w, 0.f, 1.f)*(s.max-s.min);
                            if(s.val == (float*)&cfg.pCount) cfg.pCount = (int)v; else *s.val = v;
                        }
        }

        time += 0.016f;
        rot += 0.2f * cfg.rotSpeed; // Rotazione Tao indipendente dalle particelle

        float cx = scW/2.0f, cy = scH/2.0f;
        float r = std::min(scW, scH) / 4.5f; // Un po' più piccolo per lasciare spazio alle particelle

        pool.update(cx, cy, r);

        SDL_SetRenderDrawColor(ren, 5, 5, 8, 255);
        SDL_RenderClear(ren);

        // 1. PARTICELLE (DIETRO) - Rendering ottimizzato
        SDL_SetTextureBlendMode(texGlow, SDL_BLENDMODE_ADD);

        // Modulazione colore particelle (Ciano/Bianco)
        SDL_SetTextureColorMod(texGlow, 180, 230, 255);

        for(int i=0; i<cfg.pCount; ++i) {
            const auto& p = particles[i];
            if(p.life > 0) {
                // Alpha basato sulla vita
                Uint8 a = (Uint8)(p.life * 200 * cfg.glowInt);
                if(a>0) {
                    SDL_SetTextureAlphaMod(texGlow, a);
                    // Dimensione fissa + random (stile C)
                    float s = p.size * 15.0f;
                    SDL_FRect d = {p.x-s/2, p.y-s/2, s, s};
                    SDL_RenderTexture(ren, texGlow, nullptr, &d);
                }
            }
        }

        // 2. ORBITING LIGHTS (WISPS)
        // Aggiungiamo le luci orbitanti richieste
        for(int i=0; i<4; ++i) {
            float ang = (time * 0.8f * cfg.rotSpeed) + (i * M_PI / 2.0f);
            // Orbitano appena fuori dal raggio particelle
            float wr = r * 1.8f;
            float wx = cx + cosf(ang) * wr;
            float wy = cy + sinf(ang) * wr;

            if(i%2==0) SDL_SetTextureColorMod(texGlow, 100, 200, 255); // Blu
            else       SDL_SetTextureColorMod(texGlow, 255, 150, 100); // Arancio

            SDL_SetTextureAlphaMod(texGlow, (Uint8)(200 * cfg.glowInt));
            float ws = 60.0f;
            SDL_FRect wrct = {wx-ws/2, wy-ws/2, ws, ws};
            SDL_RenderTexture(ren, texGlow, nullptr, &wrct);
        }

        // 3. AURA ETEREA (DIETRO AL TAO)
        SDL_SetTextureColorMod(texGlow, 50, 100, 200);
        SDL_SetTextureAlphaMod(texGlow, (Uint8)(100 * cfg.glowInt));
        float ar = r * 3.5f; // Aura grande
        SDL_FRect arR = {cx-ar/2, cy-ar/2, ar, ar};
        SDL_RenderTexture(ren, texGlow, nullptr, &arR);

        // 4. TAO (SOPRA TUTTO)
        SDL_FRect tr = {cx-r, cy-r, r*2, r*2};
        SDL_RenderTextureRotated(ren, texTao, nullptr, &tr, rot, nullptr, SDL_FLIP_NONE);

        // 5. UI
        for(const auto& s : sliders) {
            SDL_SetRenderDrawColor(ren, 40,40,40,200); SDL_RenderFillRect(ren, &s.r);
            SDL_FRect f = s.r; f.w = std::max(4.f, s.r.w * (*s.val-s.min)/(s.max-s.min));
            SDL_SetRenderDrawColor(ren, s.col.r, s.col.g, s.col.b, 200); SDL_RenderFillRect(ren, &f);
            SDL_SetRenderDrawColor(ren, 150,150,150,255); SDL_RenderRect(ren, &s.r);
        }

        SDL_RenderPresent(ren);
    }

    SDL_DestroyTexture(texGlow); SDL_DestroyTexture(texTao);
    SDL_DestroyRenderer(ren); SDL_DestroyWindow(win); SDL_Quit();
}
