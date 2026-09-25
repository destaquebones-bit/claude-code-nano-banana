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

- **Spectral Sun** (nome artístico) · nome real **Jhonatan Mandato** · fundador
  da **Delírios Records** · formado DJ/produtor 2014, engenheiro de áudio 2016
  (Beatport). Lançamentos na SNOE: **Dat Beat EP (SNOE096: "Dat Beat",
  "Alright")** e **"90's" (SNOEflakes #17, SNOE109)**.
  - Diz que **Hot Since 82 tocou uma faixa dele na Argentina** (17–19/09/2026:
    Las Palapas/Potrerillos, Metropolitano/Rosário, Mandarine Park/BA Open to
    Close). Qual faixa: não confirmado. Rastreio em
    `claude.ai/artifact/UaTuisHWMuAnnzHW3EbZ4d`. Quando chegar clipe,
    confirmar por correlação contra o arquivo dele — não por ouvido.
    Sets de setembro com tracklist pública (não abertas daqui): Mixmag Lab
    03/09, Essential Mix 05/09, Obsessions 09/09, Capital Dance 12/09.
  - **Registro confirmado por gravação de tela (21/09): Josh Gigante tocou
    "My Things" (My Things EP, Otherwise Records, 2024) em 3:45:00 do set
    "All Night Long at fabric London — 02.02.24"** (5:05:08, SoundCloud
    joshgigante) — quarta hora de um all-night-long, horário de pico.
    **Erro meu corrigido:** eu tinha apontado o set "WSA x fabric 05/06/2026"
    e lido 3:45 como minutos; a ligação com Hot Since 82 naquela noite não
    existe. **Provado por correlação (21/09):** clipe de 18 s contra o master
    → NCC 0,843 (controle negativo contra Take Your Time V4: 0,173, razão
    4,9×); picos secundários em +4/+8/+12/+48/+52/+56 compassos (estrutura
    de loop da faixa); trecho = **1:15 do master** (primeiro drop); DJ tocou
    0,6–0,8% mais rápido. Outro lançamento: "Maybe Less" (Otherwise, 2025).
    Outros sets públicos do Josh para varrer: Arodes b2b Josh @ Pacha
    24/09/2024 (SoundCloud arodes10, tracklist na 1001TL e set79), Diynamic
    Festival 07/06/2025, WSA x fabric 05/06/2026; playlist "What's On My USB"
    (Spotify, 56 faixas) — conferir se "My Things" está nela. Para varrer um
    set inteiro: pedir MP3 mono 32 kbps (≈14 MB/h) e deslizar o master.
    Master "My Things" medido: −8,37 LUFS, **+1,67 dBTP** (MP3), PSR 8,31,
    f0 48,9 Hz, 2f0 +4,5 — 126 BPM. Está dentro da faixa das três referências
    em loudness e PSR; é a referência interna dele de "faixa que DJ toca".
- **Capacidade nova (21/09): verificar se um clipe é uma faixa dele.**
  `scratchpad/match.py`: log-mel 40 bandas (60 Hz–8 kHz), tira timbre médio
  e nível por quadro, NCC deslizante, varre razão de velocidade 0,975–1,025.
  Sempre rodar um **controle negativo** (mesmo clipe contra outra faixa no
  mesmo BPM) e reportar a razão; sem isso o número não vale. Em tech house
  o offset é ambíguo por múltiplos de 4 compassos — reportar o melhor e a
  periodicidade, não fingir precisão de compasso.
  - **Andrez Marques NÃO é ele** — é o artista com quem fez a collab THAT'S
    FREE. Eu tinha registrado errado como "créditos como Andrez Marquez";
    corrigido em 21/09. Nunca assinar e-mail ou crédito com esse nome.
- **Tech house / minimal deep tech**, tipicamente **126 BPM**
- Alvo declarado: **Solid Grooves, Hot Creations, Hellbent**
  - Segundo alvo, trazido depois: **Hot Since 82 / Knee Deep In Sound** — house
    mais fundo, quente e atmosférico que o lado seco. **Nenhuma referência desse
    lado foi medida ainda**; a biblioteca de três só calibra o lado Solid
    Grooves. Pedir uma faixa do Knee Deep antes de dar número para esse alvo.
  - Terceiro alvo citado (19/09): **masters de Andreas Henneberg / selo SNOE**.
    **Ele JÁ LANÇOU na SNOE** (achado por busca, 21/09): "90's" abre a
    SNOEflakes #17 (SNOE109), e há um SNOE Affairs com DJ set e entrevista
    dele. Henneberg já masterizou uma faixa dele — pedir o pré-master e a
    versão lançada da "90's" para medir a cadeia dele por subtração.
    Nenhuma faixa da SNOE foi medida ainda; não afirmar nada sobre a cadeia.
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

### As outras frentes dele (outros branches deste repositório — lidos em 21/09)

Não existe skill "carreira"; o que existe está espalhado nestes branches.
Quando ele disser "veja a skill de carreira", é isto:

- **`origin/main` → `reddit-growth/LOG.md`** (866 linhas, entradas diárias
  18/08–17/09 de uma rotina automática de pesquisa e rascunho, sem postar).
  Faixa em promoção: **"Mind Gone State"** (free download Hypeddit
  `hypeddit.com/iel6vv`, SoundCloud). Subs: r/PromoteYourMusic (usado, 1×/sem),
  r/House (reciprocidade obrigatória), r/edmproduction e r/TechnoProduction
  (só feedback thread); r/Techhouse morto; r/TechHouseBrasil fundado por ele e
  **banido em horas** (conta de 0 dias). Regra 90/10, 1 ação/dia.
  **Fila de vídeos de fã do Hot Since 82 tocando "90'S":** 1) Illusions
  Audio-Visual Festival, Hangar, Belgrado (13–15/03/2026); 2) Space of Sound,
  LAB theCLUB, Madri; 3) UNVRS, Ibiza (data incerta); 4) Toronto (evento não
  identificado). **Ou seja: o Hot Since 82 toca "90'S" há meses — a faixa da
  Argentina é quase certamente "90'S".** Só uma aprovação dele registrada
  (20/08); a rotina nunca soube o que foi postado de fato.
- **`origin/main` → `plugins/audio-analyzer/`**: plugin de marketplace com
  Essentia (chave/BPM/loudness), **modelos MusiCNN de danceability e mood**
  (`models/*.pb`, ~5 MB, no repo), Demucs, espectrograma, `compare.py`,
  `critique_gemini.py`, e **snapshots do Beatport Top 100** (`reference/
  tech-house-2026.md`, `house-2026.md`, 01/08/2026): tech house 128–130 BPM,
  52% menor, labels em alta Black Book, Hellbent, Nu Moda, Defected, After
  Midnight, Nervous, Toolroom. Item 5 do roadmap (danceability) já tem modelo
  pronto ali — testar antes de escrever outro.
- **`origin/claude/home-studio-structure-analysis-0wfzwn` →
  `home-studio/BRIEFING.md`**: acústica do estúdio dele. Sala 2,75 × 3,70 ×
  2,65 m; **modos de largura e altura empilham em 62–65 Hz; RT em 63 Hz =
  1,66 s** contra 0,24–0,47 acima de 250 Hz. Dois bass traps feitos; quatro
  membranas sintonizadas em 63 Hz projetadas, não construídas. Yamaha HS,
  MiniFuse 2, ECM8000, REW, SoundID. **Explica por que ele mixa em fone** — e
  por que o grave dele (kick e baixo em 54–58 Hz) mora bem abaixo do modo da
  sala: nos monitores, 63 Hz ressoa 1,66 s. Ele **tem um EP saindo pela
  Elevation**. Mesmo branch: `loja/` (PA da loja AXOZEE) e `quarto/`.
- **`origin/claude/axozee-shirt-designs-o8vshs` → `axozee/`**: marca de
  streetwear **AXOZEE** dele ("Asfalto × Selva"), sócio da **Destaque Bonés e
  Confecções** (daí o usuário GitHub destaquebones-bit). Rotina diária
  "Compradores de Bonés e Brindes" ainda ativa (10h UTC, Sonnet).
- **`origin/claude/claude-md-docs-tlf8ml`**: CLAUDE.md do marketplace
  (documentação técnica do plugin), sem contexto do produtor.

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

**Correção (25/09): o kick não "sai" do outro.** Medido pico a pico na V4 —
presente e regular até o fim (6:44), **−8,8 a −9,4 dB** abaixo do groove de
forma bem uniforme (dois blips curtos de −26 a −28 dB são preenchimento
rítmico entre batidas, não o kick principal). O que eu chamei de "kick
continua fora" nas três entradas abaixo (v2/v3/V4) era leitura de RMS de
bloco, que mistura o kick mais baixo com o resto caindo junto — reduz, não
apaga. Ver "Como corrigir a si mesmo".

**v3 = "Take your Time v2" (18/09)**, mesmo comprimento e loudness. Subtração
direta test 1 → v2: 400–900 Hz **+1 a +2,5 dB** em quase toda a faixa (picos em
1:15, 2:45, 4:00–4:15); 250–320 Hz −1; 1,6–4 kHz −0,7; outro ganhou chimbal
(+2,3 dB >3 kHz) mas **kick continua fora** (grave −22,7); break 3 ganhou
grave (+2,2). Resultado vs faixa: 635 Hz −8,4 → −6,6 → **−5,2**; 504 Hz
**−1,3**; 317 Hz **+2,8**; 40 Hz **−10,5 igual**; TP **−0,11** (piorou de novo,
ceiling não foi mexido). 10 de 29 dentro. Pendentes: ceiling −1,2, sub, kick
no outro, mais 3 dB em 500–650.

**V4 (19/09) — 14 de 29 dentro, recorde de qualquer faixa dele.** Subtração
direta v2 → V4: sub de 25–48 Hz entrou **só nos builds** (+15,6 dB em
1:15–1:30, +8 em 4:00–4:30), não no groove — 40 Hz −10,5 → **−6,5**, 31 Hz
**−4,6**; elemento de 400–900 Hz agora por seção (+4 dB em 1:45, 2:30–3:15,
5:15–5:30) — 504 Hz **dentro**, 635 **−4,3**, 500–630 ocupação −8,1 (era
−16,8); topo 4–16 kHz −1,5 em tudo — 5–10 kHz **dentro** (+0,1 a +0,6),
sobra 3–4 kHz +2 e 12,8/16 kHz +2; kick/baixo −0,8 dB geral; break 3 encheu
(−16,9 dBFS); contrastes dos drops 4,0 / 4,8 / 4,1; TP **−0,27**; LUFS −9,94,
PSR 10,67; 2f0/3f0 −6,2/−1,0. **Kick no outro continua fora (3ª vez).**
Pendentes por tamanho: 635 Hz, 40 Hz no groove, 3–4 kHz, 317–400, kick no
outro, ceiling.

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

## Estado da faixa FEEL SO RIGHT

6:06, **126 BPM**, **192 compassos exatos**. Faixa nova, perfil diferente de
tudo medido até agora — **primeira vez com pico de confiança real na
tonalidade**: afinação +0 cents, score 0,052 (as outras davam 0,008–0,023,
fraco demais pra confiar). **D menor**, r=0,846 — a correlação mais forte já
vista. Kick e baixo convergem em 37–38 Hz, D1, o próprio tom da faixa.

| Medida | Valor | Referências |
|---|---|---|
| LUFS integrado | −10,75 | −7,6 a −8,9 |
| True peak | −0,24 | alvo −1,0 — acima |
| PSR | 11,75 | 8,1 a 9,6 |
| Bandas dentro | **8 de 29** | HISTORY 6 · PAIN v1 10 · THAT'S FREE 12 · TYT V4 14 |
| Sub 20–60 Hz | **15,3%** | 34,2–53,7% — **muito abaixo**, padrão novo |
| 250–500 Hz | **34,8%** | maior desvio de qualquer faixa medida |
| Ducking | **8,8 dB, 80% do beat** | refs 1,5–5,1 — bem mais fundo, craft não erro |

**Perfil é o espelho do que as faixas anteriores mostravam:** ali sobrava
grave, aqui falta (25–200 Hz consistentemente 3–9 dB abaixo da faixa,
50 Hz em −9,3). 250–500 Hz +5,2 (317 Hz) e +2,7 (400 Hz) acima — maior
excesso já medido nessa região. 2–4 kHz −7 a −8 dB abaixo (buraco de
presença); 5–16 kHz **dentro** quase todo — o topo, problema recorrente
antes, aqui está certo.

**Harmônicos do baixo** (leitura agregada, a linha troca de nota o tempo
todo): 2f0 +11,4 dB, 6f0 +15,1 dB acima da "fundamental" — mas o excesso em
250–500 Hz é confirmado por medida de banda bruta, independente de nota,
então é real mesmo sem certeza da causa (baixo, pad, ou kick empilhados).

**Notas do baixo, 517 detectadas:** cluster dominante D1 36,7 Hz (42%), F1
43,7 (22%), G1 49,0 (18%), D#1 38,9 (5%) — bassline real, não erro de
oitava (confirmado testando energia em f0/2). Cluster secundário 90–110 Hz
aparece só em dois trechos (fim do intro ~17–30s, break 1 ~1:31–2:00) —
parece um padrão de baixo diferente (arpejo/riser) nos buildups, não erro.

**Kick no outro (corrigido em 25/09, ver "Como corrigir a si mesmo"): não
some.** Presente e regular até o fim, ~13 dB abaixo do groove por RMS de
bloco — mas pico a pico ele está lá o tempo todo, só mais baixo.

Estrutura: intro 16c (0:00–0:30), break 16c (1:31–2:01), break grande 24c
(3:18–4:03) com reconstrução em três estágios (~20s), dip curto 7c
(4:35–4:49), outro 15c. Contraste dos três drops: 2,5–3,5 dB total, **6–9 dB
no grave** — bom.

Relatório completo — `claude.ai/artifact/42MCR9S6CnEN3JLpPHPksL`.

**Renomeada "So Right" (v2, 25/09).** Mesmo arranjo, 365,7s, D menor — revisão
de mixagem. **13 de 29 dentro** (era 8). Bass 60–120 Hz subiu +0,6 a +2,0 dB
em toda a faixa (mais em breaks/outro); 79/100 Hz quase dentro (−0,9/−1,4,
era −3,3/−3,4). 2,5–6,4 kHz subiu +0,5 a +0,7, buraco de presença encolheu.
250–500 Hz caiu um pouco (share 34,8%→30,3%) mas **continua o maior desvio**
(317 Hz +4,9, 400 Hz +2,8). **Sub piorou** — 25 Hz −10,2 (era −9,1), share
20–60 Hz caiu de 15,3% para 12,6%. TP −0,30 (melhorou), LUFS/PSR quase
iguais, ducking igual (7,9 dB/82%).

**V3 (25/09).** Ele corrigiu: o kick **não some** no outro — pico a pico,
presente e regular até 6:05, ~9 a 13 dB abaixo do groove, controlado e
uniforme (ver "Como corrigir a si mesmo"). **13 de 29 dentro** (igual à v2).
Subtração v2→v3: **1 a 1,8 kHz acima subiu +1 a +1,8 dB em quase tudo** —
buraco de presença de 2–6 kHz **praticamente fechado**, quase tudo "dentro"
agora; sub 31,5–63 Hz subiu +1 a +1,5; 79/100 Hz desceram −1,3/−1,7 (bom,
estavam acima da faixa). **317 Hz continua o maior desvio** (+7,4, quase
sem mudar) — ainda não resolvido. Comparação completa na mesma URL acima.

## Como corrigir a si mesmo

Este projeto já teve várias conclusões derrubadas pela medição seguinte. Isso é o
funcionamento correto, não um problema. Quando acontecer, **diga na cara** e
corrija o documento publicado em vez de deixar a recomendação velha de pé.

Casos reais, para não repetir:

- Recomendei **cortar 79–100 Hz com EQ**. Errado — era a fundamental do baixo
  morando ali. Cortar teria apagado o grave. O problema era registro, não EQ.
- Estimei o **ducking em 5,1 dB** pela mixdown. Com o stem isolado eram **22,4**.
- Estimei o **decaimento do kick em 29 ms** pela mixdown. Isolado, **106 ms**.
- Repeti quatro vezes ("kick sumiu/ausente/praticamente silenciado no outro")
  que o kick desaparecia no final da Take Your Time e da Feel So Right/So
  Right, baseado em **RMS médio por bloco de 4–16 compassos** numa banda
  passa-faixa. Ele apontou o erro (25/09): medi pico a pico, sincronizado na
  batida, e o kick **está presente o tempo todo até o fim**, com espaçamento
  regular — só **9 a 13 dB mais baixo** que no groove principal, uma redução
  controlada e uniforme (não some, não é ruído de fundo). Confirmado nas
  duas faixas com o método certo. **RMS de bloco conflita "mais baixo" com
  "sumiu"** — para presença de kick, sempre checar pico a pico com
  `find_peaks` na envelope, sincronizado à batida, e comparar a altura
  mediana dos picos, não a média de energia do bloco.
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
- **WebSearch funciona; WebFetch não** (21/09): a busca roda do lado do
  servidor e devolve resumos; abrir páginas (snoemusic.com, labelradar.com,
  labelsbase.net, findmylabels.com, technoairlines.com) é bloqueado pelo
  proxy. Para pesquisa de selos/mercado, usar só a busca e marcar a fonte.
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
- Take Your Time, todas as versões (PAIN → test 1 → v2 → V4) — `claude.ai/artifact/CwfbRQfPRQR4Yf9AAJwGzA`
- Take Your Time, plano da V4 até a entrega (fases, plugins, alvos, formato) — `claude.ai/artifact/Hifg1ZmuC6qmoAUVjQYZDX`
- Selos para a demo (canais, fontes, ondas de envio, e-mail) — `claude.ai/artifact/1NmeMHNifJ9qbDn4CcpSPi`
- Feel So Right, análise completa — `claude.ai/artifact/42MCR9S6CnEN3JLpPHPksL`

Ao publicar revisão de um desses, **republicar no mesmo caminho de arquivo** para
manter a URL, em vez de criar artefato novo.

## Plugin no repositório

`techhouse-duo/` é o **SS BUMBO**, VST3 dele: dois modos (Bass e Kick) que trocam
análise por um barramento entre instâncias. Todos os parâmetros nascem em zero —
carregar não muda o som até mexer num knob. Ver `techhouse-duo/README.md`.

Ao recomendar correção de mixagem, considerar se um módulo do SS BUMBO resolve —
mas **só quando resolver melhor** que a alternativa, nunca por ser nosso.
