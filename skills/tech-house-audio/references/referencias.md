# Biblioteca de referência — tech house / minimal deep tech, ~126 BPM

Três lançamentos do nicho, medidos com `scripts/analisa_audio.py`. Use os três
juntos. Calibrar contra um sozinho já produziu dois exageros documentados no fim
deste arquivo.

## Os três

| Faixa | f0 do baixo | Melhor harmônico | Ducking | LUFS | True peak | PSR |
|---|---|---|---|---|---|---|
| Detlef — Swagon (Ralf rmx) | 41,7 Hz | −5,4 dB | 1,5 dB | −8,90 | +0,78 | 8,6 |
| Flashing Lights (original) | 46,9 Hz | −7,2 dB | 2,4 dB | −7,92 | +1,69 | 9,6 |
| Joshwa — Get Stupid | 51,0 Hz | −1,1 dB | 5,1 dB | −7,62 | +1,69 | 8,1 |

"Melhor harmônico" é o parcial mais forte acima da fundamental, relativo a ela.

## Onde os três concordam — estes são alvos utilizáveis

**Fundamental do baixo: 41,7 a 51,0 Hz.** Um intervalo de 9,3 Hz entre três
discos independentes. É o número mais firme desta biblioteca, e o mais útil: um
baixo fora dele por quase uma oitava explica sozinho vários sintomas ao mesmo
tempo — sub fraco, região de 79–100 Hz estufada, e 500 Hz–2 kHz vazio.

**Ducking: 1,5 a 5,1 dB.** Todos rasos. Os três põem o baixo **na mesma oitava
do kick** e separam por dinâmica — sustentado contra transiente — não por
frequência. Sidechain profundo é o contrário do que este gênero faz.

**PSR: 8,1 a 9,6 dB.** Dinâmica de master.

**Loudness: −7,6 a −8,9 LUFS.** Alto, como club master.

## Onde discordam — não invente alvo aqui

**Qual harmônico do baixo é forte.** Joshwa tem 2f0 em −13,3 dB, mais fraco que
muita faixa amadora, e compensa com 3f0 em −1,1 — quase tão alto quanto a
fundamental. Detlef e Flashing Lights concentram no segundo. Os três só
concordam em ter **algum** parcial forte; qual, é escolha.

**Topo.** Detlef é bem mais escuro que os outros dois. Calibrar brilho contra
ele sozinho produz recomendação de escurecer 5 a 8 dB quando contra os três a
diferença é de 0,2 a 4,0.

**Distribuição sub/bass.** 20–60 Hz varia de 34,2% a 53,7% da energia total.

## True peak: os três são inutilizáveis como alvo

Todos passam de 0 dBTP, dois em exatamente +1,69. São rips recodificados, e
recodificar infla o pico entre amostras. **O alvo continua −1,0 dBTP**, por
causa da conversão com perda das lojas — razão que independe das referências.

## Como usar a faixa de tolerância

Em vez de "você está +11 dB acima do Detlef", calcule o mínimo e o máximo dos
três em cada terço de oitava e reporte a distância até essa faixa. Muda o
resultado de forma significativa: pontos que pareciam fora contra uma referência
caem dentro da faixa dos três, e os desvios que sobram são poucos e reais.

Renderize como área sombreada com a curva do produtor por cima — é a imagem que
comunica mais rápido que qualquer tabela.

## Selos e o que não serve

Alvo típico deste perfil: **Solid Grooves, Hot Creations, Hellbent**.

**Defected não serve** como referência para esse alvo — é house vocal
mainstream, com estética de master oposta, mais brilhante e mais comprimida.
Se aparecer como referência, avise antes de usar.

## Dois exageros que calibrar contra uma referência só produziu

Guardados porque a causa é a mesma e ela se repete.

**"O baixo é quase uma senoide pura."** Contra Detlef sozinho parecia abismo.
Contra os três, o baixo em questão estava 2,4 dB fora da faixa — pouco. E o
Joshwa tinha o segundo harmônico *pior*.

**"O topo está 5 a 8 dB brilhante demais."** Contra os três eram +0,2 a +4,0.
Detlef é simplesmente o mais escuro dos três.

Nos dois casos o sintoma era real e a magnitude estava inflada. Uma referência
dá direção; três dão escala.
