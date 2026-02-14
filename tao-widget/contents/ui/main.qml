import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtWebEngine
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PlasmaComponents
import org.kde.kirigami as Kirigami

PlasmoidItem {
    id: root
    
    width: Kirigami.Units.gridUnit * 20
    height: Kirigami.Units.gridUnit * 20
    
    preferredRepresentation: fullRepresentation
    
    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground
    
    // Proprietà configurabili
    property int particleCount: plasmoid.configuration.particleCount
    property int rotationSpeed: plasmoid.configuration.rotationSpeed
    property bool clockwise: plasmoid.configuration.clockwise
    
    fullRepresentation: Item { 
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumWidth: Kirigami.Units.gridUnit * 15
        Layout.minimumHeight: Kirigami.Units.gridUnit * 15
        
        Item {
            id: circularContainer
            anchors.centerIn: parent
            width: Math.min(parent.width, parent.height)
            height: width
            
            // Contenitore circolare trasparente senza bordo
            Rectangle {
                id: clipCircle
                anchors.fill: parent
                radius: width / 2
                color: "transparent"
                clip: true
                
                WebEngineView {
                    id: webView
                    anchors.fill: parent
                    backgroundColor: "transparent"
                    
                    Component.onCompleted: {
                        updateHtml()
                    }
                    
                    function updateHtml() {
                        const particles = root.particleCount
                        const speed = root.rotationSpeed / 1000.0
                        const direction = root.clockwise ? 1 : -1
                        
                        loadHtml(generateHtml(particles, speed, direction), "file:///")
                    }
                    
                    Connections {
                        target: root
                        function onParticleCountChanged() { webView.updateHtml() }
                        function onRotationSpeedChanged() { webView.updateHtml() }
                        function onClockwiseChanged() { webView.updateHtml() }
                    }
                    
                    function generateHtml(particleCount, rotationSpeed, direction) {
                        return '<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Mystical Tao</title>
    <style>
        body {
            margin: 0;
            padding: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            background: transparent;
            overflow: hidden;
        }
        
        canvas {
            filter: contrast(1.1);
        }
    </style>
</head>
<body>
    <canvas id="taoCanvas"></canvas>
    
    <script>
        const canvas = document.getElementById("taoCanvas");
        const ctx = canvas.getContext("2d", { alpha: true });
        
        const PARTICLE_COUNT = ' + particleCount + ';
        const ROTATION_SPEED = ' + rotationSpeed + ';
        const ROTATION_DIRECTION = ' + direction + ';
        
        function resizeCanvas() {
            canvas.width = window.innerWidth;
            canvas.height = window.innerHeight;
        }
        
        resizeCanvas();
        
        let centerX = canvas.width / 2;
        let centerY = canvas.height / 2;
        let radius = Math.min(canvas.width, canvas.height) / 4;
        
        let rotation = 0;
        let time = 0;
        
        class Particle {
            constructor() {
                this.reset();
            }
            
            reset() {
                const angle = Math.random() * Math.PI * 2;
                const distance = radius + Math.random() * 100;
                this.x = centerX + Math.cos(angle) * distance;
                this.y = centerY + Math.sin(angle) * distance;
                this.vx = (Math.random() - 0.5) * 0.5;
                this.vy = (Math.random() - 0.5) * 0.5;
                this.life = 1;
                this.decay = 0.002 + Math.random() * 0.003;
                this.size = Math.random() * 2 + 0.5;
            }
            
            update() {
                this.x += this.vx;
                this.y += this.vy;
                this.life -= this.decay;
                
                if (this.life <= 0) {
                    this.reset();
                }
            }
            
            draw() {
                ctx.save();
                ctx.globalAlpha = this.life * 0.6;
                ctx.fillStyle = "#ffffff";
                ctx.shadowBlur = 10;
                ctx.shadowColor = "#88ccff";
                ctx.beginPath();
                ctx.arc(this.x, this.y, this.size, 0, Math.PI * 2);
                ctx.fill();
                ctx.restore();
            }
        }
        
        const particles = Array.from({ length: PARTICLE_COUNT }, () => new Particle());
        
        function drawTao() {
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            
            ctx.fillStyle = "rgba(0, 0, 0, 0.05)";
            ctx.fillRect(0, 0, canvas.width, canvas.height);
            
            particles.forEach(particle => {
                particle.update();
                particle.draw();
            });
            
            ctx.save();
            ctx.translate(centerX, centerY);
            ctx.rotate(rotation);
            
            const pulse = Math.sin(time * 0.05) * 0.2 + 0.8;
            
            ctx.save();
            ctx.globalAlpha = 0.3 * pulse;
            ctx.strokeStyle = "#88ccff";
            ctx.lineWidth = 2;
            ctx.shadowBlur = 20;
            ctx.shadowColor = "#88ccff";
            ctx.beginPath();
            ctx.arc(0, 0, radius + 40, 0, Math.PI * 2);
            ctx.stroke();
            ctx.restore();
            
            const glowGradient = ctx.createRadialGradient(0, 0, 0, 0, 0, radius);
            glowGradient.addColorStop(0, "rgba(136, 204, 255, 0.2)");
            glowGradient.addColorStop(1, "transparent");
            ctx.fillStyle = glowGradient;
            ctx.beginPath();
            ctx.arc(0, 0, radius + 20, 0, Math.PI * 2);
            ctx.fill();
            
            const whiteGradient = ctx.createRadialGradient(0, 0, 0, 0, 0, radius);
            whiteGradient.addColorStop(0, "#ffffff");
            whiteGradient.addColorStop(1, "#e0e0e0");
            
            ctx.beginPath();
            ctx.arc(0, 0, radius, 0, Math.PI * 2);
            ctx.fillStyle = whiteGradient;
            ctx.fill();
            
            const blackGradient = ctx.createRadialGradient(0, 0, 0, 0, 0, radius);
            blackGradient.addColorStop(0, "#1a1a1a");
            blackGradient.addColorStop(1, "#000000");
            
            ctx.beginPath();
            ctx.arc(0, 0, radius, -Math.PI / 2, Math.PI / 2, false);
            ctx.arc(0, radius / 2, radius / 2, Math.PI / 2, -Math.PI / 2, true);
            ctx.arc(0, -radius / 2, radius / 2, Math.PI / 2, -Math.PI / 2, false);
            ctx.closePath();
            ctx.fillStyle = blackGradient;
            ctx.fill();
            
            ctx.save();
            ctx.shadowBlur = 15;
            ctx.shadowColor = "#ffffff";
            ctx.fillStyle = "#ffffff";
            ctx.beginPath();
            ctx.arc(0, -radius / 2, radius / 7, 0, Math.PI * 2);
            ctx.fill();
            ctx.restore();
            
            ctx.save();
            ctx.shadowBlur = 10;
            ctx.shadowColor = "#000000";
            ctx.fillStyle = "#000000";
            ctx.beginPath();
            ctx.arc(0, radius / 2, radius / 7, 0, Math.PI * 2);
            ctx.fill();
            ctx.restore();
            
            ctx.save();
            ctx.globalAlpha = 0.3 * pulse;
            ctx.strokeStyle = "#88ccff";
            ctx.lineWidth = 1;
            ctx.shadowBlur = 10;
            ctx.shadowColor = "#88ccff";
            ctx.beginPath();
            ctx.arc(0, 0, radius, -Math.PI / 2, Math.PI / 2, false);
            ctx.arc(0, radius / 2, radius / 2, Math.PI / 2, -Math.PI / 2, true);
            ctx.arc(0, -radius / 2, radius / 2, Math.PI / 2, -Math.PI / 2, false);
            ctx.stroke();
            ctx.restore();
            
            ctx.save();
            ctx.strokeStyle = "#444";
            ctx.lineWidth = 3;
            ctx.shadowBlur = 15;
            ctx.shadowColor = "#88ccff";
            ctx.beginPath();
            ctx.arc(0, 0, radius, 0, Math.PI * 2);
            ctx.stroke();
            ctx.restore();
            
            ctx.save();
            ctx.globalAlpha = 0.4;
            for (let i = 0; i < 4; i++) {
                const angle = (time * 0.02) + (i * Math.PI / 2);
                const x = Math.cos(angle) * (radius + 25);
                const y = Math.sin(angle) * (radius + 25);
                
                ctx.fillStyle = i % 2 === 0 ? "#88ccff" : "#ffaa88";
                ctx.shadowBlur = 20;
                ctx.shadowColor = ctx.fillStyle;
                ctx.beginPath();
                ctx.arc(x, y, 3, 0, Math.PI * 2);
                ctx.fill();
            }
            ctx.restore();
            
            ctx.restore();
        }
        
        function animate() {
            rotation += ROTATION_SPEED * ROTATION_DIRECTION;
            time++;
            drawTao();
            requestAnimationFrame(animate);
        }
        
        window.addEventListener("resize", () => {
            resizeCanvas();
            centerX = canvas.width / 2;
            centerY = canvas.height / 2;
            radius = Math.min(canvas.width, canvas.height) / 4;
            particles.forEach(p => p.reset());
        });
        
        animate();
    </script>
</body>
</html>'
                    }
                }
            }
        }
    }
}
