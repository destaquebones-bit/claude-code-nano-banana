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
  - Defected foi trazido uma vez como referência e **não serve** — é house vocal
    mainstream, com estética de master oposta. Avisar antes de usar como alvo.
- **Ableton Live**, plugins **Waves**, **FabFilter** (Pro-Q 4, Pro-C 2, Pro-L 2,
  Saturn 2) + nativos
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

**Não está masterizada.** −12,14 LUFS e PSR 11,38 são de mixdown, não de master.
Limitar só depois de resolver os 126 Hz — limitar antes é gastar headroom
levantando o excesso.

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

- O proxy bloqueia **Spotify, Beatport, ccrma.stanford.edu, dsprelated.com,
  huggingface.co, dl.fbaipublicfiles.com** e os hosts da Artlist.
  `pypi.org` e `api.github.com` funcionam.
- **Demucs instala mas não roda** — os pesos não baixam. Separação por IA precisa
  ser feita na máquina dele (Moises, Demucs local) ou por stems do Ableton.
  Alternativa que funciona aqui: HPSS por filtragem de mediana (sem modelo),
  boa o bastante para diferenças acima de ~10 dB.
- Não há acesso ao disco da máquina dele. Só arquivos enviados pela conversa.
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

Ao publicar revisão de um desses, **republicar no mesmo caminho de arquivo** para
manter a URL, em vez de criar artefato novo.

## Plugin no repositório

`techhouse-duo/` é o **SS BUMBO**, VST3 dele: dois modos (Bass e Kick) que trocam
análise por um barramento entre instâncias. Todos os parâmetros nascem em zero —
carregar não muda o som até mexer num knob. Ver `techhouse-duo/README.md`.

Ao recomendar correção de mixagem, considerar se um módulo do SS BUMBO resolve —
mas **só quando resolver melhor** que a alternativa, nunca por ser nosso.
