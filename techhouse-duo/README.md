# TechHouse Duo

Um plugin VST3, dois modos que **conversam entre si**: uma instância no kick, uma
no bass, e elas trocam análise diretamente — sem rotear sidechain no Ableton.

Feito pra resolver um problema específico: **bass abafado em tech house**.

---

## Por que um plugin só e não dois

Dois `.vst3` separados são dois binários carregados separadamente, cada um com
sua própria cópia de qualquer estado estático — eles **não conseguiriam** enxergar
os dados um do outro. Um binário só, com um seletor de Modo, faz as duas
instâncias viverem no mesmo módulo e compartilharem de verdade o barramento de
comunicação (`Source/dsp/LinkBus.h`). A restrição técnica definiu o formato do
produto, não o contrário.

## Como usar no Ableton

1. Instância no canal do **kick** → Modo `Kick`, Link `A`
2. Instância no canal do **bass** → Modo `Bass`, Link `A`
3. Pronto. O painel de diagnóstico mostra `Link A: connected` nos dois.

Sem sidechain, sem roteamento. Se você preferir usar sidechain mesmo assim (ou
não tiver instância no kick), o modo Bass aceita entrada de sidechain como
alternativa — o ducking espectral continua funcionando, só perde a precisão de
timing que o Link dá.

Até 8 pares independentes podem coexistir (`Link A` até `H`).

---

## As quatro causas de "bass abafado" e o que ataca cada uma

Os alvos de frequência vêm do documento de referência deste próprio repositório
(`plugins/audio-analyzer/reference/tech-house-2026.md`): kick com fundamental em
50-60Hz, bass com fundamental 50-80Hz e harmônicos até ~300-400Hz.

| Causa | Módulo | O que faz de diferente |
|---|---|---|
| Ressonância de nota | `SpectralTamer` (grade harmônica) | Corta "o 4º harmônico da nota atual", não "250 Hz" — o corte acompanha a linha de baixo enquanto ela se move |
| Notas desiguais ("one-note bass") | `NoteLevelCompensator` | Aprende o nível de cada nota MIDI ao longo do loop e nivela sozinho |
| Mascaramento kick↔bass | `KickDucker` | Abaixa só as bandas que o kick ocupa, no instante exato — preserva o corpo em 200-400Hz que o sidechain full-band destrói |
| Falta de tradução em caixas pequenas | `HarmonicExciter` | Harmônicos exatos da nota, sem intermodulação |

### O detector de ressonância

Um som natural cai de forma suave de harmônico pra harmônico — em dB, quase uma
reta. Então em vez de comparar cada banda contra um limiar fixo (o que um EQ
dinâmico comum faz), comparamos contra a **média dos dois vizinhos**. Uma banda
em cima da curva natural dá excesso zero, não importa o volume; uma banda que
salta acima dos vizinhos é ressonância. É isso que torna o detector independente
de nível e de material.

### O exciter sem lama

Um saturador distorce o sinal **inteiro**, então cada par de parciais gera também
tons de soma e diferença. Esses produtos de intermodulação não são harmônicos da
nota e são uma das principais causas do bass soar embolado em vez de maior.

Aqui a fundamental é primeiro isolada até virar quase uma senoide pura, e só ela
é moldada. Uma senoide sozinha não tem com o que intermodular, e passá-la por
**polinômios de Chebyshev** dá exatamente o n-ésimo harmônico: `T2(sin)` → 2f0,
`T3(sin)` → 3f0, e assim por diante. Verificado por teste: energia nos harmônicos
inteiros, nada em 2.5×f0.

### Link bidirecional

- **Kick → Bass**: perfil espectral em 8 bandas + força do transiente, carimbado
  com a posição da timeline
- **Bass → Kick**: a fundamental que está sendo rastreada, pra o knob
  `Bass-Aware` notchar exatamente a nota do bass em vez de um corte fixo que só
  serve pra uma nota da linha

Os frames são endereçados por **posição na timeline do host**, não por "o mais
recente". Isso torna o link imune à ordem em que o Live processa as tracks — o
bass pega o dado do kick pertencente exatamente às amostras que está trabalhando.

---

## Interface

Três visualizadores, todos alimentados por dados que o DSP já calculava e antes
jogava fora — o plugin sabia qual harmônico estava cortando e reportava só um
número.

- **Espectro harmônico** — grade em escala log com as barras posicionadas nos
  harmônicos da nota rastreada. As barras **se movem com a linha de baixo**; o
  corte aplicado é desenhado em vermelho, retirado do topo da barra. A zona de
  lama (200-400Hz) fica marcada permanentemente.
- **Medidor de ducking** — uma barra por banda do Link. Mostra a energia do kick
  em cinza e o abaixamento em vermelho, deixando visível que só as bandas onde o
  kick vive são puxadas pra baixo.
- **Mapa de notas** — desvio de nível de cada nota em relação à média da linha.
  Barra alta e vermelha = a nota que está saindo do lugar. É o problema do
  "one-note bass" mostrado como figura em vez de um número.

Knobs, painéis e indicadores são **desenhados em código** (`ui/AnalogLookAndFeel.h`),
não bitmaps: ficam nítidos em qualquer escala de tela e em qualquer tamanho de
janela, não pesam no binário, e todos os controles ficam consistentes entre si —
nada disso vale para imagens de knob geradas ou fotografadas, que saem numa
resolução fixa e precisam ser casadas à mão.

## Drive no kick (opcional, desligado por padrão)

O único estágio de saturação analógica do plugin, e só no modo Kick. No bass
seria contraproducente: saturação **adiciona** harmônicos, muitos deles em
150-400Hz, que é exatamente a faixa que este plugin existe para limpar.

No kick não há esse conflito, e o desenho é restrito de propósito:

- **Só a banda de corpo.** O sub fica intocado (harmônicos ali cairiam na zona
  de lama), e o click também (é onde aliasing seria audível, e forçaria
  sobreamostragem e latência num modo que hoje tem zero). O corpo é onde mora o
  punch, e seus 2º/3º harmônicos ficam bem abaixo de Nyquist.
- **Assimétrico**, com deslocamento de ponto de operação constante — é assim que
  uma válvula ou transformador polarizado se comporta, e é o que gera harmônicos
  pares além dos ímpares do tanh.
- **Nivelado em loudness**: a compensação é *medida* rodando uma senoide de
  referência pela curva a cada mudança de parâmetro, não estimada. Sem isso,
  subir o Drive só deixaria mais alto — e mais alto sempre ganha num A/B.
- **Transparente em zero**, por blend explícito.

## Limitações declaradas

- **Latência de 40ms no modo Bass** (a 48kHz). Rastrear pitch em 40-80Hz exige
  uma janela longa; o áudio é atrasado por esse tanto pra que a estimativa de
  nota se alinhe com o áudio que ela descreve. O host compensa isso na
  reprodução. Modo Kick tem latência zero.
- **Quando o rastreador não tem confiança**, os cortes note-aware desaparecem
  gradualmente (não travam numa grade errada). O knob `Mud (fixed)` roda numa
  grade fixa e continua trabalhando — é a degradação graciosa, não mágica.
- **Monofônico**. Baixo com acorde ou duas notas simultâneas não é rastreável por
  este método. Tech house é quase sempre monofônico, por isso a aposta.
- Não é modelagem de circuito nem cópia de nenhum plugin comercial.

---

## Instalar sem compilar (recomendado)

O GitHub Actions compila pra macOS e Windows a cada push
(`.github/workflows/build-plugins.yml`).

1. Aba **Actions** do repositório → clique na execução mais recente do workflow
   "Build plugins"
2. Role até **Artifacts** → baixe `techhouse-duo-macOS` ou `techhouse-duo-Windows`
3. Descompacte (tem um `.zip` dentro do `.zip` que o GitHub gera)
4. Copie o `TechHouse Duo.vst3` pra pasta de plugins:
   - **macOS**: `~/Library/Audio/Plug-Ins/VST3/`
   - **Windows**: `C:\Program Files\Common Files\VST3\`

**macOS — passo obrigatório.** Arquivo baixado da internet entra em quarentena do
Gatekeeper e o Live não carrega (build feito localmente não tem esse problema).
Rode uma vez depois de copiar:

```bash
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/"TechHouse Duo.vst3"
```

O build do macOS é **universal** (arm64 + x86_64), então funciona com o Live
nativo em Apple Silicon e sob Rosetta.

## Build local

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
```

No macOS, para gerar universal como o CI faz:
`-DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"`

O JUCE é baixado sozinho na primeira configuração. Saída:
- `build/TechHouseDuo_artefacts/Release/VST3/TechHouse Duo.vst3`
- `build/TechHouseDuo_artefacts/Release/Standalone/TechHouse Duo`

O `.vst3` já é copiado pra pasta do sistema (`~/Library/Audio/Plug-Ins/VST3/` no
macOS, `C:\Program Files\Common Files\VST3\` no Windows, `~/.vst3/` no Linux).
No Linux, instale antes: `libasound2-dev libcurl4-openssl-dev libx11-dev
libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev
libxrender-dev libglu1-mesa-dev mesa-common-dev libfreetype-dev
libfontconfig1-dev`.

## Testes

```bash
cmake -B build-tests -S tests -DCMAKE_BUILD_TYPE=Release
cmake --build build-tests -j
./build-tests/DspTests_artefacts/Release/DspTests
```

27 checagens offline: precisão do rastreador de pitch em 7 notas (todas dentro de
2 cents), rejeição de silêncio/ruído, ausência de erro de oitava, o detector de
ressonância acertando qual harmônico, pureza harmônica do exciter, o nivelamento
por nota, e o endereçamento por posição do LinkBus.

Para ver os visualizadores com dados reais (o app standalone silencia a entrada
de áudio, então na tela eles aparecem vazios):

```bash
cmake --build build-tests --target UiPreview
./build-tests/UiPreview_artefacts/Release/UiPreview preview.png
```

Roda uma linha de baixo sintética pelos módulos de verdade e renderiza os três
visualizadores num PNG, sem precisar de display.

Esses testes já pegaram dois bugs reais. O primeiro: os envelopes de suavização rodavam a cada
bloco mas com coeficientes calculados por amostra, então um ataque de 5ms virava
2,5s num buffer de 512. `OnePoleEnvelope::processOverSamples` corrige, e há um
teste de regressão comparando buffers de 64 e 1024 amostras.

O segundo apareceu ao renderizar o medidor de ducking pela primeira vez: com o
limiar absoluto original, toda banda com energia real passava tão longe do
limiar que grudava no máximo — as oito bandas abaixavam igual, ou seja, ducking
full-band fantasiado de multibanda, justamente o que este módulo existe para
evitar. Agora o ducking segue a **forma** do espectro do kick relativa ao pico
dele (curva inversa de EQ), e três checagens travam isso: silêncio não abaixa
nada, o abaixamento difere entre bandas, e as bandas graves abaixam mais que as
agudas.

## Estrutura

```
techhouse-duo/
  Source/
    PluginProcessor.h/.cpp     # cadeia dos dois modos, lookahead, publicação no Link
    PluginEditor.h/.cpp        # UI por modo + painel de diagnóstico
    dsp/
      LinkBus.h                # barramento entre instâncias (seqlock, endereçado por posição)
      PitchTracker.h           # YIN decimado, 35-250Hz
      SpectralTamer.h          # detector de ressonância, grade re-sintonizável
      NoteLevelCompensator.h   # nivelamento por nota MIDI
      KickDucker.h             # ducking espectral por banda
      HarmonicExciter.h        # Chebyshev travado na fundamental
      KickShaper.h             # transiente + encurtamento do rabo de grave
      BandAnalyzer.h           # medição das 8 bandas publicadas
      DspCommon.h, ParameterIDs.h
  tests/DspTests.cpp
```
