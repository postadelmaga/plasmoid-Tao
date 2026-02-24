#version 450

layout(location = 0) in  vec4 v_color;
layout(location = 0) out vec4 fragColor;

void main()
{
    // 1. OTTIMIZZAZIONE ALFA: Se la particella sta morendo ed è 
    // praticamente invisibile, ferma subito il calcolo.
    if (v_color.a < 0.01) {
        discard;
    }

    vec2 uv = gl_PointCoord - vec2(0.5);
    
    // 2. OTTIMIZZAZIONE MATEMATICA: Usa il prodotto scalare (dot) invece 
    // di length(). length() forza la GPU a calcolare una radice quadrata, 
    // che è un'operazione lenta. Il dot() ottiene la distanza al quadrato (distSq).
    // Moltiplichiamo per 4.0 in modo che il bordo del cerchio sia esattamente a 1.0.
    float distSq = dot(uv, uv) * 4.0;

    // 3. OTTIMIZZAZIONE GEOMETRICA (La più importante per le performance):
    // Elimina fisicamente i pixel che si trovano negli angoli del quadrato
    // del Point Sprite. Questo risparmia circa il 21% di lavoro alla GPU
    // per ogni singola particella.
    if (distSq > 1.0) {
        discard;
    }

    // Calcola il glow. Siccome distSq equivale già a (dist * dist)
    // della tua formula originale, lo inseriamo direttamente.
    float glow = exp(-distSq * 3.0);

    // Colore finale
    vec4 finalColor = v_color * glow;

    // 4. ULTIMO CONTROLLO: Se il glow ha reso il pixel quasi del tutto 
    // trasparente verso i bordi, non inviarlo al motore di blending.
    if (finalColor.a < 0.01) {
        discard;
    }

    fragColor = finalColor;
}