# Product Guidelines - Mystical Tao

## Visual Identity
- **Cyber-Zen Aesthetics:** Blend traditional Zen symbolism (Tao) with modern, neon, high-contrast digital aesthetics. Use dark backgrounds with glowing, neon accents (e.g., cyan, magenta, and lime) for the clock and particle highlights.
- **Minimalist Nature:** Maintain a clean, unobtrusive layout with soft colors and organic particle movements, even within the Cyber-Zen theme. The central Tao symbol should be the visual anchor.
- **Contrast and Clarity:** Ensure that the neon clock hands and particle system are always clearly visible against the central Tao symbol and various desktop backgrounds.

## User Experience (UX)
- **Fluid & Smooth Interactions:** All animations, including the clock's appearance/disappearance and particle transitions, MUST use easing curves (e.g., `Quintic.InOut`) for a high-quality, premium feel.
- **Reactive Design:** The particle system SHOULD react to user interactions like mouse movements or clicks, creating a more engaging and interactive desktop experience.
- **Customization First:** Users should be able to easily adjust visual parameters (density, colors, clock visibility) through a standard KDE Plasma configuration dialog that adheres to system design patterns.

## Performance
- **Zero Impact Background:** When the widget is not visible or the desktop is inactive, the particle simulation should minimize resource usage.
- **C++ Core:** Heavy computations (like individual particle physics) MUST be handled by the C++ plugin to maintain a high, jitter-free frame rate.
