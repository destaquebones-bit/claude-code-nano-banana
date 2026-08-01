# Tech House — benchmark de mercado (snapshot 2026-08-01, Beatport Top 100)

Coletado navegando no chart público do Beatport (beatport.com/genre/tech-house/11/top-100),
lendo os metadados que a propria pagina expoe (titulo, artista, label, BPM, tom, data de
lancamento). Nao foi feito download nem analise de audio dos previews — Beatport nao expoe
os arquivos de audio pra scraping e isso violaria os termos de uso da plataforma. Esse
documento combina os dados quantitativos coletados com conhecimento geral de producao/mix/
master de tech house, para servir de checklist ao revisar faixas do usuario.

**Isto e um snapshot, nao uma verdade permanente.** O chart muda toda semana. Antes de usar
esses numeros pra dar feedback importante numa faixa, considere re-coletar o chart atual se
esse arquivo tiver mais de ~2-3 meses.

Ver tambem [[house-2026.md]] — mesma metodologia aplicada ao genero House. As duas tem BPM
parecido (~128-130), mas diferem bastante em harmonia (Tech House ~52% menor, House ~82% menor).

## O que os dados mostram

### BPM
- Faixa dominante: **128-130 BPM** (52% do chart), com uma cauda relevante em **131-133**
  (17%, puxando pro lado mais peak-time/energico).
- Media geral: 128.5 BPM. Praticamente nada abaixo de 122 ou acima de 133 (as excecoes sao
  classicos "evergreen" fora da curva: RUFUS DU SOL "Innerbloom" a 106 BPM, e "Badman Sound"
  a 156 BPM que e cruzamento com breakbeat/UKG, nao tech house convencional).
- **Implicacao pratica**: se uma faixa sua estiver fora de 124-133 BPM, ela sai do que o
  mercado atual espera de "tech house" — nao e proibido, mas vale ser intencional sobre isso.

### Tom / harmonia
- **52% menor / 48% maior** — bem equilibrado, com leve preferencia por tom menor (consistente
  com o groove mais hipnotico/dark do genero).
- Tons mais comuns: **F, G, A, C** — tons "amigaveis" pra transposicao de vocal chops e samples,
  nada muito extremo em bemois/sustenidos.

### Labels em alta (mais faixas no chart)
Black Book Records, Hellbent Records, Nu Moda, Defected, AFTER MIDNIGHT, Nervous Records,
range., THRIVE MUSIC, Maccabi House. Vale estudar o catalogo recente dessas labels como
referencia de som e como alvo de demo submission.

### Artistas recorrentes no chart
San Pacho, Mau P, Chris Lorenzo, Matroda, After Midnight, Chico Rose, Dale Howard, Ammo Avenue,
Rafael, Adam Ten. Referencias diretas de producao atual do genero.

### Atualidade do chart
85% das faixas sao de 2026 — o chart e dominado por lancamento recente, nao por classicos.
As poucas excecoes antigas (FISHER "Losing It" 2018, Kollektiv Turmstrasse 2016, RUFUS DU SOL
2016) sao hinos atemporais que se sustentam por serem extremamente tocados em DJ sets, nao
porque o som antigo "ainda funciona" sem ajuste.

### Sinal de fusao Latin Tech
Pelo menos 3 faixas marcadas com subgenero "Latin Tech" (percussao latina — congas, bongos,
guiro — sobre a base de tech house). Confirma o que ja apareceu nos packs de sample do
usuario (Afrobeat Percussion, Latin Tech Terrace etc.) — e uma direcao de mercado real, nao
so gosto pessoal do usuario.

## Cross-check: Traxsource Tech House Top 100 (mesmo snapshot 2026-08-01)

Coletado do mesmo jeito (beatport.com -> traxsource.com/genre/18/tech-house/top, so leitura
da pagina). Diferenca importante: **a listagem do Traxsource nao expoe BPM/tom por faixa**
(so aparece na pagina individual de cada musica) — coletar isso pras 100 faixas exigiria 100
visitas de pagina, o que e scraping excessivo pra um dado que ja temos via Beatport. Entao
essa secao cobre so label/artista, nao BPM/tom.

- **Labels em alta no Traxsource** (diferentes do topo do Beatport): Nervous, Toolroom/Toolroom
  Trax, Little Big Groove Records, Black Box Underground, Hellbent Records, Defected, Wh0 Plays.
  Nervous e Toolroom aparecem fortes nos dois sites — sao labels "seguras" de referencia.
- **Artista fora da curva**: **Rick Silva** aparece **7 vezes** no top 100 do Traxsource
  (posicoes 2, 25, 36, 39, 41, 46, 96) — muito mais que qualquer outro artista em qualquer um
  dos charts vistos ate agora. Pode ser um produtor extremamente prolifico e em alta agora, ou
  reflexo de estrategia de lancamento (varias faixas seguidas do mesmo artista sobem chart de
  plataformas menores mais facil do que no Beatport). Vale ouvir o catalogo dele pra entender
  o que esta acertando, mas nao tratar como "o som definitivo do genero" so por volume.
- **Overlap de faixas entre plataformas**: varias faixas aparecem nos dois charts (ex: "Badman
  Sound" Hannah Wants/Trace, "Gator Boots", "Beat Goes On", "Bounce UR Body", "$MONEY", "Blow Ya
  Mind", "Take Our Time") — quando uma faixa aparece TOP nos dois sites ao mesmo tempo, e sinal
  mais forte de que realmente esta em alta no genero (nao so favorecida pelo algoritmo de uma
  plataforma so).
- **Alerta de confiabilidade de metadado**: a faixa "Badman Sound" (Hannah Wants, Trace, label
  Nervous) aparece no Beatport tageada como **156 BPM - C Major**, mas na pagina de faixa do
  Traxsource aparece como **125 BPM - Cmin (C menor)** — mesma musica, mesma label, mesma data
  de lancamento (2026-07-17), metadado de BPM e tom **divergente** entre as duas plataformas.
  Isso e um lembrete real: nao confiar cegamente no BPM/tom que uma unica plataforma mostra —
  quando possivel, validar com `analyze.py report` (analise de audio de verdade) em vez de so
  copiar o metadado da loja.

## Benchmark de loudness MEDIDO (7 faixas de referencia do usuario, coletado 2026-08-01)

Diferente do resto deste documento (que e conhecimento geral + dados de chart), isto foi
**medido de verdade** via `scripts/analyze.py` (Essentia `LoudnessEBUR128` + `TruePeakDetector`,
padrao EBU R128 / ITU-R BS.1770) em 7 faixas de referencia que o usuario forneceu (compras dele
no Beatport, artistas: Alaia & Gallo, Andreas Henneberg/Beth Lydi, Beltran (BR), Bonafique, OMRI.,
Rafael, SCRIPT). Trata isso como o benchmark mais confiavel deste documento — atualizar se o
usuario mandar mais faixas de referencia depois.

- **LUFS integrado**: -9.35 a -8.07, media **-8.64 LUFS**. Confirma (e refina pra baixo) a
  estimativa generica de "-8 a -6 LUFS" que estava aqui antes — na pratica as faixas de
  referencia ficam mais perto de -9 a -8 do que de -6.
- **True peak**: media **+0.35 dBTP**, com **6 das 7 faixas ACIMA de 0 dBTP** (uma chegou a
  +1.43 dBTP). Isso **contradiz a regra comum de mastering de "-1dBTP pra evitar clipping
  inter-sample"** — na pratica, masters comerciais de tech house/house parecem tolerar (ou nao
  se preocupar com) true peak acima de 0dB. Nao tratar "-1dBTP" como regra rigida pra esse
  genero; ainda vale mencionar se uma faixa do usuario estiver MUITO acima disso (tipo +3dBTP+),
  mas +0.5 a +1dBTP parece normal no mercado real.
- **Loudness Range (LRA)**: 3.57 a 7.56 LU, media **5.3 LU** — dinamica controlada mas nao
  brickwalled (LRA abaixo de ~3 seria "guerra do loudness" de verdade; essas faixas tem espaco
  dinamico real, so nao sao gravacoes acusticas dinamicas).
- **BPM dessas 7 faixas**: 125-129.8, media 126.97 — consistente com o range do chart (124-133),
  mas na metade mais baixa/moderada dele.

## Convencoes de sound design / mix / master do genero
(conhecimento geral de producao eletronica, nao extraido do chart — usar como ponto de partida,
nao como regra absoluta)

- **Kick**: curto, punchy, fundamental por volta de 50-60Hz, pouco sustain — o groove vem do
  padrao ritmico, nao de um kick longo tipo house classico.
- **Bassline**: "rolling bass" e a assinatura do genero — linha de baixo em colcheias/
  semicolcheias com sidechain moderado ao kick (nao tao agressivo quanto EDM mainstage),
  fundamental 50-80Hz com harmonicos ate ~300-400Hz pra ter corpo em sistemas de club menores.
- **Percussao**: hats/shakers/congas em groove sincopado, geralmente com swing sutil (nao
  quantizado 100% reto) — e o que da o "balanco" caracteristico do tech house sobre o house
  classico mais reto.
- **Vocal chops**: extremamente comuns (a maioria das faixas do chart tem "feat." ou vocal
  sample picado) — geralmente processados com pitch/formant shift, side-chained levemente,
  usados como elemento ritmico/textural mais do que "vocal principal".
- **Arranjo**: intro/outro longos (16-32 compassos) pensados pra mixagem de DJ, breakdown
  central curto, poucos elementos tocando ao mesmo tempo (groove minimalista) comparado a
  outros generos de dance.
- **Mix**: geralmente mais headroom e dinamica do que EDM mainstage — nao e brickwall.
- **Masterizacao**: pensada pra streaming/pool de DJ, nao pra "vencer a guerra do loudness" —
  over-compression mata o groove que e a propria razao de ser do genero. **Ver a secao de
  benchmark medido acima pros numeros reais de LUFS/true peak/LRA** — nao usar "-1dBTP" ou
  "-8 a -6 LUFS" como regra generica, os dados medidos mostram que a pratica real do mercado
  e um pouco diferente disso.

## Como usar isso ao revisar uma faixa do usuario

1. Rodar `scripts/analyze.py report --path faixa.wav` pra pegar BPM, tom e loudness reais.
2. Comparar BPM/tom contra as faixas dominantes acima — nao pra forcar conformidade, mas pra
   o usuario saber se esta dentro ou fora da curva atual (e decidir se isso e intencional).
3. Comparar loudness (`loudness` do relatorio Essentia) contra a referencia de -8 a -6 LUFS —
   se a faixa estiver muito mais alta/comprimida, vale mencionar.
4. Para sound design/mix qualitativo (groove do kick, textura do bass, uso de vocal chops),
   usar as convencoes acima como checklist de perguntas a fazer sobre a faixa, ja que nao ha
   como "ouvir" a faixa de fato nesta sessao — pedir pro usuario descrever ou tocar a faixa
   junto se precisar de feedback mais fino.
