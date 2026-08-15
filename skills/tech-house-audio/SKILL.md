---
name: tech-house-audio
description: Análise técnica e perceptiva de faixas, mixdowns e stems de música eletrônica — LUFS, true peak, balanço tonal contra referências, fundamental e harmônicos do baixo, profundidade real de sidechain, mascaramento por bandas críticas — mais edição de MIDI de baixo (gate, oitava, acento). Use SEMPRE que um arquivo de áudio (.wav .aiff .flac .mp3 .ogg) ou MIDI for enviado, e sempre que a conversa envolver mixagem, masterização, kick, bass, sub, sidechain, true peak, loudness, "faixa abafada", "não traduz em caixa pequena", comparação com referência, ou preparo de demo para selo. Rode a análise antes de responder qualquer coisa sobre o áudio — opinar sobre som sem medir é o erro que esta skill existe para evitar.
---

# Análise de áudio para música eletrônica

## A regra que define tudo

**Meça antes de afirmar.** Quando chegar áudio, rode o analisador antes de
escrever qualquer frase sobre como a faixa está. Não porque medir seja mais
importante que ouvir, mas porque é a única metade que você consegue fazer.

**Você não escuta.** Diga isso sempre que a conversa começar a tratar medição
como julgamento estético. Nenhum número diz se a música é boa, se o groove
balança, ou se o arranjo prende. Fingir o contrário é o pior resultado possível
aqui — pior que não responder.

## Rodar

```bash
python3 scripts/analisa_audio.py FAIXA.wav
python3 scripts/analisa_audio.py MINHA.wav --ref REFERENCIA.wav
python3 scripts/analisa_audio.py bass.wav --contra kick.wav    # mascaramento
python3 scripts/groove_midi.py entrada.mid saida.mid --oitava -1 --gate 1.15
```

Dependências: `pip install numpy scipy soundfile pyloudnorm mido`.
Lê WAV, AIFF, FLAC, MP3 e OGG.

## As duas camadas, e por que a distinção não pode borrar

**Física** — LUFS, true peak, espectro, fase. Exata e reprodutível. Responde
*quanto mede*. Nunca responde *como soa*.

**Perceptiva** — bandas críticas na escala Bark, mascaramento, sonoridade em
sones. É **modelo**, não medida. Existe porque o ouvido faz três coisas que um
espectro em dB ignora: soma energia dentro de bandas críticas, espalha cada
banda sobre as vizinhas (que é o mascaramento), e comprime o resultado por uma
potência de ~0,23.

Rotule qual camada produziu cada número. As duas discordam com frequência, e a
discordância costuma ser o achado mais útil: já aconteceu de a física dizer que
kick e baixo colidiam a 0,3 dB um do outro, e o modelo perceptivo mostrar zero
bandas críticas mascaradas. **Colisão espectral e mascaramento perceptivo não
são a mesma coisa.**

## Referência é calibração, não alvo

Sem referência você não tem escala, e vai inventar alvo — é a falha mais comum.
Com **uma** referência você calibra contra as escolhas daquele disco. Só com
**três ou mais** dá para separar assinatura de gênero de escolha de artista.

Nunca proponha clonar a referência. Ela serve para dimensionar *o quanto* algo
está fora, e o produtor decide o que fazer com isso.

Leia `references/referencias.md` para a biblioteca já medida (tech house,
126 BPM) e para os alvos que os discos concordam. Se o produtor trabalha noutro
subgênero, meça referências novas em vez de reusar essas.

## Armadilhas de medição, todas descobertas errando

Estas custaram conclusões erradas antes de serem entendidas. Verifique cada uma
antes de reportar o número correspondente.

**Mixdown esconde a profundidade do sidechain.** O ducking do baixo medido na
mixdown deu 5 dB; no stem isolado eram 22. O kick preenche os vales e falseia a
leitura. Ducking e decaimento só valem medidos em stem isolado.

**Mixdown falseia o decaimento do kick** pelo mesmo motivo: 29 ms na mixdown,
106 ms isolado.

**A fundamental do baixo varia ~10 Hz conforme o janelamento.** Medindo pelos
picos do próprio baixo dá um valor, pelos picos do kick dá outro. Reporte a
faixa, nunca um valor com casa decimal fingindo precisão.

**Um rastreador de pitch erra oitava para cima quando a fundamental é fraca.**
Antes de afirmar que f0 é X, confira quanta energia existe em X/2. Se estiver
25+ dB abaixo, X é mesmo a fundamental.

**Onset de conteúdo grave não se localiza melhor que ~25 ms.** Um ciclo de 50 Hz
já dura 20 ms. Números de swing e micro-timing de baixo caem dentro dessa
incerteza — se todas as faixas medidas derem o mesmo valor, é o detector, não o
groove. Não construa recomendação em cima disso.

**Stem isolado contra mixagem completa é comparação inválida.** Uma banda de
40–300 Hz numa mixagem pega kick e percussão junto. Compare stem com stem.

**Stems em MP3 não somam de volta à mixdown** (0,718 de correlação medida). Para
conferir soma, peça WAV ou FLAC.

**Rips de site inflam o true peak.** Recodificar aumenta o pico entre amostras.
Três referências medidas passaram de 0 dBTP, duas delas em exatamente +1,69 —
artefato, não prática de masterização. Nunca use o TP de um rip como alvo.

## Como reportar

Ordene por tamanho do desvio, não por ordem de descoberta. Diga contra o que
cada número foi comparado. Separe o que é medição do que é conhecimento de
produção — se você está dando conselho de craft que não mediu, diga isso.

Quando houver versão anterior, compare com ela: o que andou, o que não andou, e
o que piorou, em números. Progresso parcial merece ser dito como fração ("21%
do caminho"), não como aprovação.

## Quando a medição derrubar uma conclusão sua

Vai acontecer, e é o funcionamento correto. **Diga na cara e corrija o documento
publicado**, em vez de deixar a recomendação velha de pé em algum lugar.

Um exemplo real de por que isso importa: medindo um platô de +11 dB em 79–100 Hz,
a recomendação óbvia era cortar ali com EQ. Errado — era a fundamental do baixo
morando naquela região, e cortar teria apagado o grave. O sintoma estava certo,
a causa era registro e não EQ. **Antes de recomendar corte, verifique se o que
está sobrando é a fundamental de alguma coisa.**

## Alvos que não dependem de referência

- **True peak −1,0 dBTP.** A conversão com perda do Beatport e do Spotify
  aumenta o pico entre amostras; a −0,3 a faixa distorce depois de codificada.
  Vale mesmo que todas as referências estejam acima de zero.
- **Grave em mono.** Correlação próxima de 1,0 abaixo de 120 Hz.
- **Quatro ciclos da fundamental** para o ouvido formar altura. A 45 Hz isso são
  89 ms — uma nota de sub mais curta que isso vira baque sem nota. É o que torna
  perigoso descer o baixo de oitava sem alongar o gate.
