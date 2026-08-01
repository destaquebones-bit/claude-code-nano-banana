# House — benchmark de mercado (snapshot 2026-08-01, Beatport Top 100)

Mesma metodologia de [[tech-house-2026.md]]: navegacao read-only no chart publico do Beatport
(beatport.com/genre/house/5/top-100), so metadados expostos na propria pagina (titulo, artista,
label, BPM, tom, data). Nenhum audio baixado.

**Snapshot, nao verdade permanente** — re-coletar se este arquivo tiver mais de ~2-3 meses.

## O que os dados mostram

### BPM
- Dominante: **128 BPM** (28%) e **130 BPM** (21%) — juntos quase metade do chart.
- Faixa 128-130 cobre **57%** do chart. Media 128.8, bem proxima do Tech House (128.5) —
  os dois generos ocupam essencialmente o mesmo território de andamento.
- Outliers acima do normal: "Actin' Tough" 140 BPM, "No More" 142 BPM — tech/bass house
  cruzando pra energia mais alta, excecao, nao regra.

### Tom / harmonia — a diferenca mais marcante vs Tech House
- **82% menor / 18% maior** — MUITO mais inclinado a tom menor que Tech House (que estava
  52/48). House atual (nesse chart) e consideravelmente mais "dark"/melancolico na harmonia
  do que Tech House, que e mais equilibrado.
- **Implicacao pratica pro usuario**: se ele produz nos dois generos, vale ter isso em mente —
  uma faixa de house em tom maior tende a soar "fora da curva" do que domina o chart agora,
  enquanto em tech house isso e normal.
- Tons mais comuns: E, Ab, A, D, Bb, F — mais distribuido que Tech House (que concentrava em F/G/A/C).

### Labels em alta
LTF Records (destaque isolado, 8 faixas — vale estudar o catalogo deles de perto), CircoLoco
Records, NO ART, Defected, Factory 93 Records, ROSSI.HOME//GRXWN., Make The Girls Dance Records.

### Artistas recorrentes
Prospa (6 faixas — o mais dominante do chart), ANOTR, Franky Rizardo, Mellizos, Simon Kidzoo,
Reboot, 3DDY, Cloonee. Referencias diretas.

### Diferenca grande vs Tech House: presenca de classicos
81% das faixas sao de 2026 (parecido com Tech House), mas o House tem uma cauda relevante de
**classicos revividos**: "I feel for you" (Bob Sinclar, 2000), "Stupidisco" (Junior Jack, 2004),
"U Don't Know Me" (Armand Van Helden, 2006), "I Want Your Soul" (Armand Van Helden, 2013),
"(It Goes Like) Nanana" (Peggy Gou, 2023), alem de varios **remixes/reworks de faixas antigas**
lancados em 2026 (Groovejet remix, World Hold On Fisher Rework, Silk remix). Tech House quase
nao teve isso. **Isso sinaliza que o publico/DJs de house valorizam mais revival e reinterpretacao
de classicos do que o publico de tech house atual.**

### Fusao Latin House
"Salsa Del 78", "El Alma", "Somos Uno" marcados como subgenero "Latin House" — mesma tendencia
de fusao latina observada no Tech House ([[tech-house-2026.md]]), confirmando que e uma direcao
de mercado mais ampla, nao especifica de um genero so.

## Cross-check: Traxsource House Top 100 (mesmo snapshot 2026-08-01)

Mesmo método do cross-check de [[tech-house-2026.md]] — só metadado de label/artista, sem
BPM/tom (Traxsource não expõe isso na listagem, só na página individual da faixa).

- **Labels em alta no Traxsource House**: Fool's Paradise (4), Soulfuric Deep, There Was Jack,
  Trois Garçon, Defected, Nervous, Toolroom (3 cada). Defected, Nervous e Toolroom aparecem
  fortes nos dois sites — labels "seguras" de referência pros dois gêneros.
- **Overlap de faixas entre plataformas**: "Ritmo Da Rua" (Universal Love, Harry Romero) é #1 no
  Traxsource House e também charta no Beatport House — sinal forte de hit cross-platform.
  "Groovejet", "You Don't Want To Think About It", "Phunky" e "Je Suis Music" também aparecem
  nos dois charts.
- **Alerta de classificação de gênero inconsistente**: "I Never Knew" (Adam Ten) aparece como
  **Tech House** no Beatport (posição #3 do chart de Tech House) mas como **House** no
  Traxsource. Mesma faixa, gêneros diferentes conforme a plataforma — reforça que os limites
  entre Tech House e House são fluidos na prática, e que rótulo de gênero de uma única loja não
  é uma verdade absoluta.

## Convencoes de sound design / mix / master do genero
(conhecimento geral, complementar ao Tech House — as diferencas sao o que importa aqui)

- **Groove**: mais "classico"/organico que Tech House — menos rolling bass hipnotico, mais
  enfase em groove de bateria com influencia disco/funk (muitos titulos remetem a disco:
  "Groovejet", "Disco", "Boogie Time", "Soul Makossa").
- **Harmonia**: dado o forte viés pra tom menor no chart atual, acordes/pads em menor tendem a
  soar mais "no lugar" agora do que maior — mas isso e tendencia de chart, nao regra do genero
  em si (house classico tem muita faixa em maior tambem).
- **Vocal**: samples vocais mais "cantados"/melodicos e menos picados-como-percussao do que em
  Tech House — mais próximo de vocal de disco/soul, refletindo a raiz do genero.
- **Arranjo**: estrutura similar a Tech House (intro/outro longos pra DJ mix), mas com mais
  espaço pra elementos melódicos/harmônicos no corpo da faixa.
- **Mix/master**: mesma referência de headroom do Tech House — não é gênero de loudness war.
  Ver [[tech-house-2026.md]] pro benchmark de LUFS/true peak/LRA **medido de verdade** em 7
  faixas de referência (média -8.64 LUFS integrado, true peak médio +0.35 dBTP — a regra comum
  de "-1dBTP" não reflete a prática real do mercado nesses dois gêneros).

## Como usar isso ao revisar uma faixa do usuário

Mesmo processo de [[tech-house-2026.md]]: rodar `scripts/analyze.py report`, comparar BPM/tom/
loudness contra os números acima. Se a faixa for taggeada como "house" mas o usuário não
mencionar tom, vale checar se está em maior — nesse caso, mencionar que o chart atual pende bem
mais pra menor, não como regra, mas como dado de contexto.
