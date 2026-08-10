# Mastering Chain — analog-style VST3 for Ableton Live

Um plugin de masterização único (VST3/Standalone, feito com JUCE) que junta em
uma única inserção o que normalmente seria uma cadeia de vários plugins:
EQ com caráter analógico, compressor de bus ("glue"), saturação de fita,
alargador estéreo com segurança de grave mono, e limiter true-peak com
lookahead. Pensado pra rodar como inserção no master bus do Ableton Live.

**Estado atual**: esqueleto genérico e funcional, com DSP real (não é mock) —
compila e roda (testado neste ambiente Linux headless, ver `Build verification`
abaixo). Os valores padrão ainda **não** foram calibrados para nenhum plugin
específico do seu rack. Assim que você mandar os prints do seu rack atual
(quais plugins, em que ordem, quais parâmetros), os defaults de cada estágio
serão ajustados pra chegar perto do caráter que você já usa.

## O que cada estágio faz

Ordem do sinal: `Input → EQ → Compressor → Width → [oversampled 2x: Saturador → Limiter] → Output → Metering`

| Estágio | Arquivo | O que modela |
|---|---|---|
| Input | `PluginProcessor.cpp` | trim de ganho + inversão de fase |
| Analog EQ | `Source/dsp/AnalogEQ.h` | grave estilo Pultec (boost e cut simultâneos, diferentes frequências/Q — não cancela, "levanta e aperta" o grave), 2 sinos móveis, shelf de agudo/"air", e uma leve saturação tipo transformador ("Iron") |
| Glue Compressor | `Source/dsp/GlueCompressor.h` | compressor VCA com detecção **linkada em estéreo** (nunca processa L/R independente — evita deslocar a imagem estéreo), soft-knee, release automático dependente de programa |
| Tape Saturator | `Source/dsp/TapeSaturator.h` | waveshaping tanh assimétrico (harmônicos pares + ímpares), rolloff de agudo tipo cabeçote de fita, leve realce de grave |
| Stereo Width | `Source/dsp/StereoWidener.h` | processamento Mid/Side; abaixo da frequência de corte o canal "side" é filtrado pra mono (evita problema de fase/compatibilidade mono no grave) |
| Limiter | `Source/dsp/AnalogLimiter.h` | brickwall com lookahead (5ms), detecção de pico **linkada em estéreo**, "drive" pra empurrar mais forte contra o teto |
| Metering | `Source/dsp/LoudnessMeter.h` | LUFS momentâneo/integrado (aproximação K-weighted) + true peak estimado (interpolação 4x) |

Saturador e Limiter rodam dentro de um bloco sobreamostrado 2x
(`juce::dsp::Oversampling`, FIR half-band) — evita aliasing gerado pela
não-linearidade da saturação e deixa a detecção de true-peak do limiter mais
precisa.

**Honestidade sobre os limites**: o medidor de LUFS/true-peak aqui é uma
aproximação (boa o suficiente pra mixar/masterizar por ela, mas não é um
medidor certificado de broadcast). O algoritmo de each estágio é DSP real e
comum na indústria (tanh saturation, compressor VCA, EQ Pultec-style,
limiter lookahead) — não é modelagem de circuito componente-a-componente
como fazem plugins comerciais de ponta (Waves, UAD). Ver a resposta no chat
sobre isso: é uma cadeia honesta e funcional, não uma cópia 1:1 de nenhum
plugin comercial específico — ainda.

## Build

Requer CMake ≥ 3.22 e um compilador C++17 (g++/clang++/MSVC/Xcode). O JUCE
é baixado automaticamente via `FetchContent` na primeira configuração —
não precisa instalar nada do framework manualmente.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
```

Isso gera:
- `build/MasteringChain_artefacts/Release/VST3/Mastering Chain.vst3`
- `build/MasteringChain_artefacts/Release/Standalone/Mastering Chain` (app standalone, útil pra testar sem abrir o Ableton)

O CMake já copia o `.vst3` pra pasta de plugins do sistema
(`COPY_PLUGIN_AFTER_BUILD TRUE`):
- **macOS**: `~/Library/Audio/Plug-Ins/VST3/`
- **Windows**: `C:\Program Files\Common Files\VST3\`
- **Linux**: `~/.vst3/`

Se preferir copiar manualmente, pegue o `.vst3` do caminho de build acima e
solte na pasta correspondente.

### Dependências de sistema (Linux apenas)

No macOS/Windows o JUCE não precisa de nada além do Xcode/Visual Studio. No
Linux, antes do `cmake -B build`:

```bash
sudo apt-get install -y libasound2-dev libcurl4-openssl-dev libx11-dev \
    libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev \
    libxrandr-dev libxrender-dev libglu1-mesa-dev mesa-common-dev \
    libfreetype-dev libfontconfig1-dev
```

## Instalar e usar no Ableton Live

1. Compile (acima) — o `.vst3` já é copiado automaticamente pra pasta certa.
2. No Ableton: `Preferences → Plug-Ins`, confirme que a pasta VST3 do seu
   sistema está marcada e clique em "Rescan".
3. O plugin aparece como **"Mastering Chain"** na lista de plugins VST3.
4. Arraste pra inserção do master bus (ou de qualquer track/return).

## Estrutura do projeto

```
mastering-chain-vst/
  CMakeLists.txt
  Source/
    PluginProcessor.h/.cpp   # cadeia de processamento, parâmetros, oversampling
    PluginEditor.h/.cpp      # interface (knobs por seção + medidores)
    dsp/
      ParameterIDs.h         # todos os parâmetros automatizáveis, num lugar só
      DspCommon.h             # utilitários compartilhados (filtro stereo, envelope, soft-knee)
      AnalogEQ.h
      GlueCompressor.h
      TapeSaturator.h
      StereoWidener.h
      AnalogLimiter.h
      LoudnessMeter.h
```

## Próximos passos (depende do seu rack)

Quando os prints dos plugins que você usa chegarem, os ajustes previstos são:
- Recalibrar valores padrão (threshold/ratio do compressor, frequências do
  EQ, quantidade de drive da saturação, release do limiter) pro caráter de
  cada plugin equivalente no seu rack.
- Documentar aqui, estágio por estágio, qual plugin seu cada um substitui.
- Se algum plugin seu fizer algo que essa cadeia genérica não cobre (ex:
  multibanda, dynamic EQ, exciter harmônico específico), avaliar se vale
  adicionar um estágio novo.
