



# Earl Grey Matcha Spectral Panner

**LFO-driven spectral panner plugin** that splits audio at a moving crossover frequency, sending low and high frequency bands to opposite stereo channels. Makes any track sound wider, more immersive, and three-dimensional.

Built with JUCE/C++ by **Jiwoo Son**

Part of the **Matcha Series** — sister plugin to [Strawberry Matcha Delay](https://github.com/sonji16/strawberry-matcha-delay)

v1.0.0 

Install: Available at [Releases](https://github.com/sonji16/earl-grey-matcha-spectral-panner/releases)

Formats: VST3 and AU

---

## How It Works

The plugin splits your audio into two frequency bands at a crossover point using cascaded IIR lowpass filters. The low band (LPF) pans to one side while the high band (HPF) pans to the opposite side. An LFO continuously moves the crossover point, creating a wide stereo field where different frequencies move in opposite directions.

### Signal Flow

```
Input (stereo summed to mono)
  │
  ├──► LPF chain (12-48 dB/oct) ──► Equal power pan (L↔R) ──┐
  │                                                           ├──► Dry/Wet Mix ──► Gain ──► Output
  └──► HPF (dry - LPF) ──────────► Equal power pan (R↔L) ──┘
                    ▲
                    │
              LFO drives crossover frequency (log scale)
              Depth scales LFO modulation amount
              Width scales stereo pan spread
```

### Key DSP Details

- **Highpass via subtraction**: HPF = dry signal − lowpassed signal, ensuring the two bands always sum to unity (mono-compatible)
- **Logarithmic crossover modulation**: `cutoff = centerFreq × 2^((lfo − 0.5) × sweepOctaves × 2)` — sweeps in musical octaves, not linear Hz
- **Equal power panning**: `gain = sqrt(position)` — no volume dip at center
- **Cascaded IIR filters**: 1–4 stages of `juce::IIRFilter` in series (12–48 dB/oct)
- **LFO phase accumulator**: `phase += rate / sampleRate` with subtraction-based wrap for drift-free frequency accuracy
- **Depth and width are independent**: depth controls filter sweep range, width controls stereo spread — changing one doesn't affect the other

---

## Parameters

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| **Rate** | 0.05 – 8.0 Hz | 0.5 Hz | LFO speed |
| **Depth** | 0 – 100% | 70% | How much the LFO moves the crossover frequency |
| **Width** | 0 – 100% | 100% | How far apart L and R channels are panned |
| **Center Freq** | 100 – 8000 Hz | 1000 Hz | The center point the crossover sweeps around |
| **Sweep Octaves** | 0.5 – 4.0 | 2.0 | How many octaves above/below center the crossover travels |
| **Filter Slope** | 12 / 24 / 36 / 48 dB | 24 dB | Steepness of the LPF/HPF crossover |
| **LFO Shape** | Sine / Saw / Square / Triangle | Sine | Waveform shape of the LFO |
| **Mix** | 0 – 100% | 100% | Dry/wet blend |
| **Gain** | −12 to +12 dB | 0 dB | Output level |

### Recommended Settings

| Use Case | Rate | Depth | Width | Center Freq |
|----------|------|-------|-------|-------------|
| Subtle stereo width | 0.05 – 0.15 Hz | 30 – 50% | 80 – 100% | 800 – 1500 Hz |
| Natural movement | 0.3 – 0.8 Hz | 50 – 70% | 70 – 90% | 600 – 1200 Hz |
| Dramatic sweep | 1.0 – 2.0 Hz | 80 – 100% | 100% | 500 – 1000 Hz |
| Rhythmic panning | Sync to BPM | 70 – 90% | 100% | 800 – 1500 Hz |

For **making a track sound massive without obvious movement**: rate 0.05 Hz, depth 30-50%, width 80-100%, center freq around 1000 Hz. 
Slower rate makes it sound spatial but not sweeping.

---

## LFO Shapes

| Shape | Character |
|-------|-----------|
| **Sine** | Smooth organic breathing — most natural and massive sounding |
| **Saw** | Ramps one direction then snaps back — asymmetric chase feel |
| **Square** | Hard instant cuts between positions — rhythmic gate effect |
| **Triangle** | Constant speed linear sweep — mechanical pendulum |

---

## UI Features

- **Frequency Display**: Real-time visualization on a 20 Hz – 20 kHz log-scale axis showing the LPF (matcha green) and HPF (bergamot amber) response curves with moving crossover line, sweep range indicator, and current frequency readout
- **LFO Waveform Display**: Serum-style waveform view that shows more cycles at higher rates, with animated playhead dot, depth-responsive amplitude
- **L/R Level Meters**: Real-time peak level bars for left (matcha) and right (bergamot) channels
- **Custom LookAndFeel**: Brown-body rotary knobs with matcha green arcs, matching the Strawberry Matcha Delay visual family

---

## Installation

### macOS Installer
1. Download the `.pkg` installer from [Releases](https://github.com/sonji16/earl-grey-matcha-spectral-panner/releases)
2. Double-click to install
3. Restart your DAW
4. The plugin appears as "JiwooSonSpectralPanner" in VST3/AU plugin lists

### Manual Install
Copy the plugin files to:
- **VST3**: `/Library/Audio/Plug-Ins/VST3/`
- **AU**: `/Library/Audio/Plug-Ins/Components/`

### Supported Formats
- VST3
- AU (Audio Unit)
- macOS 10.13+

---

## Technical Notes


### Filter Slope Stages
Each `juce::IIRFilter` with `makeLowPass` is a 2nd-order (12 dB/oct) filter. Cascading N stages gives N×12 dB/oct:
- 1 stage = 12 dB/oct (gentle)
- 2 stages = 24 dB/oct (standard Linkwitz-Riley)
- 3 stages = 36 dB/oct
- 4 stages = 48 dB/oct (very steep)

### Per-Sample Coefficient Updates
Filter coefficients are recalculated every sample to track the LFO. While computationally expensive, this ensures the crossover frequency follows the LFO without artifacts or zipper noise. Future optimization could use `SmoothedValue` to update coefficients at a lower rate.

---

## The Matcha Series

| Plugin | Description |
|--------|-------------|
| [Strawberry Matcha Delay](https://github.com/sonji16/strawberry-matcha-delay) | Tempo-synced delay with EQ, tap tempo, and real-time scope |
| **Earl Grey Matcha Spectral Panner** | LFO-driven spectral panner for stereo width and movement |

---

## Credits

**Jiwoo Son** — Design, DSP, UI

- GitHub: [github.com/sonji16](https://github.com/sonji16)

Built with [JUCE](https://juce.com/) framework. C++

Font: Talina DEMO from [font designer].

