# Contexto do projeto

## Quando chegar áudio, analise sem esperar pedido

Sempre que um arquivo de áudio for enviado nesta conversa — faixa, mixdown, stem
ou referência — rode a análise antes de responder qualquer coisa:

```bash
python3 tools/analisa_audio.py FAIXA.wav
python3 tools/analisa_audio.py MINHA.wav --ref REFERENCIA.wav
python3 tools/analisa_audio.py bass.wav --contra kick.wav      # mascaramento
```

Se as dependências não estiverem no ambiente:
`pip install numpy scipy soundfile pyloudnorm`

Lê WAV, AIFF, FLAC, MP3 e OGG.

**Duas camadas, e a distinção nunca deve ser embaçada:**

- **Física** — LUFS, true peak, espectro, fase. Exato e reprodutível. Não diz
  nada sobre como soa.
- **Perceptiva** — bandas críticas, mascaramento, sonoridade em sones. É
  *modelo*, não medida. É o mais perto de escutar que dá para chegar sem ouvir.

**Eu não escuto.** Isso precisa ser dito sempre que a conversa começar a tratar
medição como julgamento estético. Nenhum número diz se a música é boa.

## O produtor

- **Spectral Sun** (nome artístico) · créditos como **Andrez Marquez**
- **Tech house / minimal deep tech**, tipicamente **126 BPM**
- Alvo declarado: **Solid Grooves, Hot Creations, Hellbent**
  - Segundo alvo, trazido depois: **Hot Since 82 / Knee Deep In Sound** — house
    mais fundo, quente e atmosférico que o lado seco. **Nenhuma referência desse
    lado foi medida ainda**; a biblioteca de três só calibra o lado Solid
    Grooves. Pedir uma faixa do Knee Deep antes de dar número para esse alvo.
  - Defected foi trazido uma vez como referência e **não serve** — é house vocal
    mainstream, com estética de master oposta. Avisar antes de usar como alvo.
- **Ableton Live**, plugins **Waves**, **FabFilter** (Pro-Q 4, Pro-C 2, Pro-L 2,
  Saturn 2) + nativos
  - Waves confirmados por uso: F6, C6, L2, WLM Plus, Scheps 73, Smack Attack,
    C1, H-Delay, R-Verb. **Não tem** SSL G-Master Buss Comp — para cola de bus
    usar o Glue Compressor nativo (mesmo modelo SSL, feito pela Cytomic).
- **Mixa em fone** (visto em captura de tela, 2026-09-17): Sonarworks/SoundID com
  Sennheiser HD 600, Audio-Technica ATH-M40x e AirPods Pro. Explica o formato
  em "V" da PAIN (200–400 estufado, 500–800 cavado, topo brilhante) e o sub
  ausente abaixo de 45 Hz — fone não entrega 40 Hz no corpo. Ao analisar,
  apontar sub e médio-grave como as regiões em que o monitoramento dele mente.
- **WLM Plus** configurado com True Peak Max −2,0, Target −24, Short Max −16
  (padrões de TV). Já sugeri −1,0 / −9 / −8.
- Escreve em português, em caixa alta, direto ao ponto. Responda em português.

### Como ele trabalha

- Itera rápido e manda revisões. Sempre compare com a versão anterior e diga o
  que andou, o que não andou e o que piorou — em números.
- Pede profundidade técnica máxima e explicitamente pede o melhor esforço.
- Aprende melhor **com imagem**: gráficos e diagramas valem mais que parágrafos.
- Quer **parâmetros exatos** (frequência, Q, ganho, ataque, release).
- **A referência é calibração, não alvo.** Ele deixou claro: a faixa dele não
  precisa ficar igual à referência, é questão de qualidade. Use referência para
  dimensionar o quanto algo está fora, nunca para propor clonagem.

## Biblioteca de referência

Três lançamentos do nicho, já medidos. **Use os três, nunca um só** — foi
calibrar contra um sozinho que produziu dois exageros meus (ver abaixo).

| Faixa | f0 do baixo | Melhor harm. | Ducking | LUFS | TP | PSR |
|---|---|---|---|---|---|---|
| Detlef — Swagon (Ralf rmx) | 41,7 Hz | −5,4 dB | 1,5 dB | −8,90 | +0,78 | 8,6 |
| Flashing Lights (orig.) | 46,9 Hz | −7,2 dB | 2,4 dB | −7,92 | +1,69 | 9,6 |
| Joshwa — Get Stupid | 51,0 Hz | −1,1 dB | 5,1 dB | −7,62 | +1,69 | 8,1 |

**Os três passam de 0 dBTP.** Não é padrão do gênero — são rips recodificados, e
recodificar infla o pico entre amostras. **Nunca use o TP deles como alvo.** O
alvo continua −1,0 dBTP por causa da conversão com perda do Beatport/Spotify.

### O que os três concordam

- **f0 do baixo: 41,7 a 51,0 Hz.** Intervalo de 9,3 Hz entre três discos
  independentes. É o número mais firme que este projeto tem.
- **Ducking: 1,5 a 5,1 dB.** Todos rasos.
- **PSR: 8,1 a 9,6 dB.**
- Eles põem o baixo **na mesma oitava do kick** e separam por *dinâmica*
  (sustentado vs. transiente), não por frequência.

### Onde eles discordam — não invente alvo aqui

- **Qual harmônico do baixo é forte.** Joshwa tem 2f0 em −13,3 (pior que o do
  produtor!) mas 3f0 em −1,1. Só concordam em ter **algum** parcial forte.
- **Topo.** Detlef é bem mais escuro que os outros dois. A faixa é larga.
- **Distribuição sub/bass.** 20–60 Hz varia de 34,2% a 53,7%.

## Estado da faixa HISTORY (v2, "NEW MIX TEST 2")

Medida contra a faixa dos três. **6 de 29 bandas dentro**, mas a maioria das que
estão fora está fora por menos de 1 dB. Os desvios reais:

| Frequência | Fora por |
|---|---|
| 100 Hz | **+8,7 dB** |
| 79 Hz | **+7,1 dB** |
| 504 Hz | **−6,0 dB** |
| 63 Hz | +5,1 dB |
| 2,5 kHz | +4,0 dB |

**Dentro da faixa:** 25, 40, 50, 159 e 200 Hz. O passa-altas que ele fez
resolveu o subsônico — esse trabalho está concluído.

### A causa raiz, confirmada por stem isolado

- **f0 do baixo: 77 a 118 Hz** (mediana 88–98 conforme o janelamento).
  **9,4 semitons acima do topo da faixa das referências.**
- **Baixo tem ~nada abaixo de 60 Hz**: −41,9 dB contra −15,4 do kick.
- **Kick e baixo no mesmo nível em 60–120 Hz**: −16,5 vs −16,2 dB.
- **Ducking de 22,4 dB** — 4,4× o mais fundo das três referências.
- **Kick: f0 54 Hz**, decai −6 dB em 106 ms, −20 dB em 158 ms. Longo.
- **504 Hz não tem dono**: baixo em −42,7 e kick em −40,1 na região 500 Hz–2 kHz.
  O buraco nunca foi cavado, nunca foi preenchido.

### Prioridades atuais

1. **Descer o baixo uma oitava** (ou camada de sub em 39–52 Hz)
2. **Reduzir o ducking** de 22,4 para 2–6 dB — só é possível depois do item 1
3. **Limiter em −1,0 dBTP** (está em +0,81)
4. **Preencher 504 Hz**
5. **Outro mixável** de 32–64 compassos (a faixa corta em ~1 s aos 5:36)

Itens rebaixados por medição: escurecer o topo, e harmônicos do baixo.

## Estado da faixa THAT'S FREE (v1, colab)

6:02, 126,8 BPM, **192 compassos exatos**. Faixa diferente da HISTORY e com
diagnóstico diferente — não reaproveite as conclusões de lá.

| Medida | Valor | Referências |
|---|---|---|
| LUFS integrado | −12,14 | −7,6 a −8,9 |
| True peak | −0,89 | alvo −1,0 |
| PSR | 11,38 | 8,1 a 9,6 |
| f0 do baixo | **57,8 Hz** | 41,7 a 51,0 |
| Ducking | **2,9 dB** | 1,5 a 5,1 — **dentro** |
| Correlação < 120 Hz | 0,999 | mono, ok |
| Bandas dentro da faixa | **12 de 29** | HISTORY tinha 6 |

**Resolvido em relação à HISTORY:** ducking dentro da faixa, fundamental na
oitava certa, 2f0 mais rico que o das três referências, **buraco de 504 Hz
fechado** (−0,0, dentro), estrutura fechada com outro despido nos últimos ~40 s.

**O que sobrou, concentrado num ponto só:**

| Frequência | Fora por |
|---|---|
| 126 Hz | **+9,5 dB** |
| 100 Hz | +9,1 dB |
| 79 Hz | +7,4 dB |
| 63 Hz | +4,8 dB |
| 25 Hz | +4,4 dB |
| 317 Hz | −2,7 dB |
| 400 Hz | −2,5 dB |

**Causa: 2f0 (≈116 Hz) está +1,2 dB MAIS ALTO que a fundamental.** Diferença
crítica em relação à HISTORY: lá a fundamental morava em 88–98 Hz e cortar
teria apagado o grave. **Aqui a fundamental está a 57,8 Hz, então cortar
110–130 Hz é seguro** — é harmônico, não fundamental. Verifique isso sempre
antes de repetir a recomendação de corte.

**Tônica: A#/Bb, modo indefinido.** A fundamental do baixo (57,8 Hz) é A#1, e o
croma corrigido põe A# em 18,8% — quase o dobro da segunda colocada. Mas a terça
maior (D, 7,1%) e a menor (C#, 7,4%) estão empatadas: **a faixa não tem terça.**
Qualquer elemento novo que toque D ou C# escolhe o modo pela produção inteira.
Tônica + quinta (A# + F) é a saída sem cor.

**Afinação global −15 cents, com confiança baixa** (o pico da curva é raso,
score 0,023). Não afirme; peça para conferir de ouvido antes de programar
sintetizador novo.

**Ocupação por banda, 250 Hz a 8 kHz** (dB abaixo da banda mais alta): a mais
vazia é **315–400 Hz (−8,7)**, seguida de 400–500 e 800–1000 (−4,7). A mais
cheia é 5–6,3 kHz (0,0) — chimbais. Elemento novo entra em 315–500 Hz, com
passa-altas em 220 Hz (fora do platô de 126 Hz) e passa-baixas em 4,5 kHz.
F4 (349 Hz) e A#4 (466 Hz) caem exatamente nos dois buracos.

**Não está masterizada.** −12,14 LUFS e PSR 11,38 são de mixdown, não de master.
Limitar só depois de resolver os 126 Hz — limitar antes é gastar headroom
levantando o excesso.

## Estado da faixa PAIN → TAKE YOUR TIME

A faixa foi renomeada: v1 = "PAIN test" (6:04), v2 = "Take Your Time test 1"
(6:45, masterizada, **216 compassos exatos**). Tabela abaixo é da v1; o que
mudou na v2:

| | v1 | v2 | Refs |
|---|---|---|---|
| LUFS | −13,31 | **−9,80** | −7,6 a −8,9 |
| PSR | 14,00 | **10,25** | 8,1 a 9,6 |
| True peak | −0,34 | **−0,18** (WLM na sessão dava −0,9: **MP3 somou 0,7 dB**) | −1,0 |
| Contraste drops 1/2/3 | 2,8 / 1,4 / 9,6 dB | **4,7 / 5,9 / 6,0** | alvo 4–6 (craft) |
| Outro | 4 s | 30 s (16 c.) **sem kick** — grave −9 dB | — |
| 635 Hz | −8,4 | −6,6 | ainda o maior desvio |
| 40 Hz | −11,7 | −10,5 | nota não decidida |
| 317 Hz | +4,9 | +3,7 | |
| 12,8 kHz | +6,1 | +3,5 | |
| Kick −6 dB | 109 ms | **51 ms** | mudou o kick ou transient shaper |
| Sub 20–60 | 24,3% | 30,3% | 34–54% |

Bandas dentro 10 → 9, mas as distâncias encolheram. Em 1:12–1:27 o grave cai
a −30 dBFS antes do drop — a técnica de contraste foi aplicada. Ceiling
recomendado: **−1,2 dBTP** por causa da inflação do MP3.

**v3 = "Take your Time v2" (18/09)**, mesmo comprimento e loudness. Subtração
direta test 1 → v2: 400–900 Hz **+1 a +2,5 dB** em quase toda a faixa (picos em
1:15, 2:45, 4:00–4:15); 250–320 Hz −1; 1,6–4 kHz −0,7; outro ganhou chimbal
(+2,3 dB >3 kHz) mas **kick continua fora** (grave −22,7); break 3 ganhou
grave (+2,2). Resultado vs faixa: 635 Hz −8,4 → −6,6 → **−5,2**; 504 Hz
**−1,3**; 317 Hz **+2,8**; 40 Hz **−10,5 igual**; TP **−0,11** (piorou de novo,
ceiling não foi mexido). 10 de 29 dentro. Pendentes: ceiling −1,2, sub, kick
no outro, mais 3 dB em 500–650.

### v1 (PAIN test) — medição original

6:04, **128 BPM** (confirmado por ele; autocorrelação deu 128,4), **192
compassos + 4,5 s de rabo** — a grade fecha. Faixa mais percussiva
que as anteriores: alinhamento tonal 0,009, o mais fraco já medido.

| Medida | Valor | Referências |
|---|---|---|
| LUFS integrado | −13,31 | −7,6 a −8,9 |
| True peak | **−0,34** | alvo −1,0 — acima |
| PSR | 14,00 | 8,1 a 9,6 |
| f0 do baixo | 53–54 Hz (A1 −35 c ou G#1 +65 c) | 41,7 a 51,0 |
| Kick, pico espectral | 53,8 Hz | mesma nota do baixo |
| 2f0 / 3f0 | −9,0 / **−2,6** | perfil Joshwa |
| Ducking | 2,3 dB | dentro |
| Bandas dentro | 10 de 29 | TF 12, HISTORY 6 |

**Resolvido:** toda a região 50–160 Hz dentro da faixa (6 de 6 bandas). O
problema de 2f0 da THAT'S FREE não existe aqui. Kick e baixo na mesma nota,
separados por dinâmica — como as referências.

**Fora, por tamanho:**

| Região | Fora por | Natureza |
|---|---|---|
| 635 Hz | **−8,4** (504 −3,1, 800 −4,2) | **arranjo**: nada mora em 500–800 Hz |
| 40 Hz | **−11,7** (31 −9,4, 25 −7,8) | **nota**: baixo em 54 Hz, refs em 42–51 |
| 317 Hz | **+4,9** (252 +3,2, 200 +2,5) | lama; fonte desconhecida pela mixdown |
| 12,8 kHz | **+6,1** (3–16 kHz todo +2 a +3,5) | contra os três, não só o Detlef |
| Outro | 4 s | sem outro — regressão vs. TF (40 s) |

Formato "V": estufado 200–400, cavado 500–800, brilhante em cima. Hipótese
(não medida): mixagem em monitor pequeno/fone.

**Afinação do sub: ressalva.** Leitura de 53–54 Hz fica 35 cents abaixo de A1.
Pode ser o rabo do kick (−20 dB em 151 ms, janela de análise começa em 130 ms)
contaminando o YIN, ou detune real. Pedir para conferir o MIDI antes de
afirmar. Se a tônica for Mi (r=0,51, fraco), E1 = 41,2 Hz cairia na faixa das
referências e resolveria o sub — decisão musical dele, não minha.

Estrutura: intro filtrada ~16 c., quebra 16 c. em 2:14, quebra **24 c.** em
3:45 (bloco a −24,7 dBFS), fim em 6:04 sem outro. **A voz mora nos dois
breaks** (2:10–2:45 e ~4:20–4:35); fora deles a transcrição não acha fala.
Letra transcrita com confiança parcial (ver relatório).

## Como corrigir a si mesmo

Este projeto já teve várias conclusões derrubadas pela medição seguinte. Isso é o
funcionamento correto, não um problema. Quando acontecer, **diga na cara** e
corrija o documento publicado em vez de deixar a recomendação velha de pé.

Casos reais, para não repetir:

- Recomendei **cortar 79–100 Hz com EQ**. Errado — era a fundamental do baixo
  morando ali. Cortar teria apagado o grave. O problema era registro, não EQ.
- Estimei o **ducking em 5,1 dB** pela mixdown. Com o stem isolado eram **22,4**.
- Estimei o **decaimento do kick em 29 ms** pela mixdown. Isolado, **106 ms**.
- Escrevi **alvos numéricos de balanço tonal inventados por mim**. Removidos.
- Reportei **mascaramento kick↔baixo como problema grave**. O modelo perceptivo
  mostrou **zero** bandas críticas mascaradas. Colisão espectral e mascaramento
  perceptivo não são a mesma coisa.
- Disse que o baixo era **"quase uma senoide pura"**. Com três referências, está
  só **2,4 dB fora** da faixa delas. Exagero causado por calibrar contra o
  Detlef sozinho.
- Disse que o topo estava **"5 a 8 dB brilhante demais"**. Contra os três é
  **+0,2 a +4,0 dB**. O Detlef é só o mais escuro dos três.

**A estimativa de f0 do baixo varia ~10 Hz** conforme o janelamento (87,8 Hz
medindo pelos picos do próprio baixo, 98,5 Hz pelos picos do kick). Reporte a
faixa, nunca um valor com casa decimal fingindo precisão.

## Limitações do ambiente

Descobertas por teste, não por suposição:

- O proxy bloqueia **Spotify, Beatport, SoundCloud (soundcloud.com, api-v2,
  sndcdn.com — testado 2026-09-17, link privado inclusive), ccrma.stanford.edu,
  dsprelated.com, huggingface.co, dl.fbaipublicfiles.com** e os hosts da Artlist.
  `pypi.org` e `api.github.com` funcionam.
- **Demucs instala mas não roda** — os pesos não baixam. Separação por IA precisa
  ser feita na máquina dele (Moises, Demucs local) ou por stems do Ableton.
  Alternativa que funciona aqui: HPSS por filtragem de mediana (sem modelo),
  boa o bastante para diferenças acima de ~10 dB.
- Não há acesso ao disco da máquina dele. Só arquivos enviados pela conversa.
- **O scratchpad e os uploads não sobrevivem a reinício de sessão.** As três
  referências em áudio se perderam; a faixa de tolerância (min/max por terço
  de oitava) foi recuperada do relatório THAT'S FREE publicado e está no
  array `D` daquele artefato (colunas: fc, valor, min, max). Para reanalisar
  referências em áudio, pedir os arquivos de novo.
- **Transcrição de voz funciona** — `pip install sherpa-onnx` e pesos do Whisper
  pelos releases do GitHub (que passam no proxy):
  `github.com/k2-fsa/sherpa-onnx/releases/download/asr-models/sherpa-onnx-whisper-small.tar.bz2`
  (640 MB; o base de 200 MB alucina em loop). O que funcionou melhor em mixdown:
  canal mid, passa-banda 250 Hz–5 kHz, janelas de 14 s com passo de 7, idioma
  fixo. HPSS antes da transcrição **piorou**. `openaipublic.azureedge.net`
  (pesos do openai-whisper) e alphacephei (Vosk) estão bloqueados.
- Para caber no limite de upload: **FLAC mono 16 bits, 60 s do groove
  principal** ≈ 2 MB e não perde nada relevante.
- Stems em MP3 **não somam de volta à mixdown** (correlação 0,718 medida). Para
  conferir soma, pedir WAV/FLAC.

## Roadmap do analisador

Cada item deve ser **medido antes de ser adotado** — o padrão deste projeto é
rejeitar o que não se comprova.

1. **Mascaramento no tempo.** Hoje roda sobre o espectro médio, o que apaga o
   mascaramento instantâneo do transiente. Maior limitação atual.
2. **Aspereza / dissonância sensorial** (Plomp-Levelt). Quantifica "embolado" —
   o mais perto de medir o que ele chama de bass abafado.
3. ~~Bandas de tolerância com várias referências~~ — **feito**, três referências
   na tabela acima. Renderiza como área sombreada no gráfico.
4. **Sonoridade ao longo do tempo** (sones a curto prazo) em vez de só a média.
5. **Consistência nota a nota** — detectar one-note bass.
6. **Análise de transiente por stem** — separar o que o limiter fez do que a
   fonte já era.

## Relatórios publicados

- Análise inicial vs. Detlef — `claude.ai/code/artifact/4f3ef9bf-3762-49a9-b374-0d070fa23f98`
- Guia de correção passo a passo (Waves + nativos, com parâmetros) — `.../8bc8637c-4fc2-4a63-b386-555b4f9bb55b`
- v1 vs v2, o que mudou — `.../eedfb972-4551-4ef8-a232-c4d31607a27c`
- Diagnóstico do baixo — `.../6e8aff75-6bab-4181-affb-460eab1a6d0b`
- Bandas de tolerância dos três — `.../488e96cf-438c-4bd5-bd20-27655fb51dc4`
- THAT'S FREE v1, análise completa — `.../bf0af573-48c4-46f3-ae57-66fe1cf50ad9`
- THAT'S FREE, arp seco (sintetizador, patch, lugar no espectro) — `.../19c4f463-435c-400a-b0bf-e716c9e263c1`
- Racks por grupo, Waves + FabFilter, alvo Knee Deep (craft, sem medição) — `claude.ai/artifact/6Up6uKTbKMhGqxe3WSX2p9`
- PAIN (test), análise completa + ideias de sound design — `claude.ai/artifact/Hcb7pMJy1mQ9JvwxoGLUSV`
- Take Your Time, v1 vs v2 — `claude.ai/artifact/CwfbRQfPRQR4Yf9AAJwGzA`

Ao publicar revisão de um desses, **republicar no mesmo caminho de arquivo** para
manter a URL, em vez de criar artefato novo.

## Plugin no repositório

`techhouse-duo/` é o **SS BUMBO**, VST3 dele: dois modos (Bass e Kick) que trocam
análise por um barramento entre instâncias. Todos os parâmetros nascem em zero —
carregar não muda o som até mexer num knob. Ver `techhouse-duo/README.md`.

Ao recomendar correção de mixagem, considerar se um módulo do SS BUMBO resolve —
mas **só quando resolver melhor** que a alternativa, nunca por ser nosso.
