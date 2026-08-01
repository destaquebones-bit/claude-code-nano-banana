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
  Referencia comum: masters ficam por volta de **-8 a -6 LUFS integrado**, true peak em
  -1dB, deixando espaco pro DJ misturar sem distorcer.
- **Masterizacao**: pensada pra streaming/pool de DJ, nao pra "vencer a guerra do loudness" —
  over-compression mata o groove que e a propria razao de ser do genero.

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
