# Briefing — home studio do Jhonatan (Spectral Sun)

> **Para o Claude que receber este arquivo.** Isto é um repasse de um projeto de
> acústica que já está em andamento há semanas. Leia tudo antes de responder
> qualquer coisa. O objetivo imediato é **conduzir a segunda medição no REW e
> analisar os arquivos `.mdat` que o Jhonatan vai mandar**. Não recomece o
> projeto do zero e não refaça diagnósticos já fechados — eles estão na seção 4
> e custaram caro.

---

## 1. Quem é e como trabalhar com ele

- **Jhonatan Mandato**, produtor e DJ brasileiro, nome artístico **Spectral Sun**.
  Faz tech house e deep house. Tem um EP saindo pela Elevation.
- Escreve em **português**, muitas vezes em CAIXA ALTA, com pressa e erros de
  digitação. Responda em português.
- **Constrói de verdade.** Ele serra, parafusa, mede e manda foto. Não trate
  como pergunta teórica: ele vai executar o que você disser.
- **Corrige você quando você erra**, e já estava certo pelo menos uma vez
  (seção 5). Leve a sério o que ele observa na sala.
- Gosta de número e de desenho. Uma tabela com medidas vale mais que três
  parágrafos de explicação.

### Regras de conduta que valeram até aqui

1. **Nunca invente um diagnóstico bonito.** Duas vezes foi construída uma
   explicação elaborada e errada. Quando a evidência for fraca, diga que é fraca.
2. **Separe o que é medido do que é previsto.** Sempre.
3. **Não prometa acessar links externos.** O ambiente bloqueia Google Drive
   (`connect_rejected`). Arquivo só chega por anexo na conversa.
4. **Corrija-se de forma seca** quando errar, e siga em frente.

---

## 2. A sala

| | |
|---|---|
| Largura | **2,75 m** |
| Comprimento | **3,70 m** |
| Pé-direito | **2,65 m** |
| Volume | **26,96 m³** |
| Piso | cerâmica, com tapete na área de escuta |
| Paredes | alvenaria pintada |
| Frequência de Schroeder | **≈ 496 Hz** — abaixo disso a sala é modal, e RT60 é aproximação grosseira |

### Modos axiais

| eixo | dimensão | 1ª | 2ª | 3ª |
|---|---|---|---|---|
| largura | 2,75 m | **62,4 Hz** | 124,7 | 187,1 |
| altura | 2,65 m | **64,7 Hz** | 129,4 | 194,2 |
| comprimento | 3,70 m | 46,4 Hz | 92,7 | 139,1 |

**Os modos de largura e altura empilham em 62–65 Hz.** É a origem de todo o
problema desta sala.

---

## 3. Equipamento

- Interface **Arturia MiniFuse 2**
- Microfone de medição **Behringer ECM8000** (condensador, precisa de +48 V)
- Monitores **Yamaha HS** sobre espumas de isolação na mesa
- MacBook, **REW** (Room EQ Wizard)
- **SoundID Reference** instalado — *tem de estar desligado durante qualquer medição*
- Arquivo de calibração: `home-studio/ECM8000_generico.cal` no repositório.
  **É a curva de outra unidade do mesmo modelo, não da unidade dele.** Corrige a
  tendência do modelo, não o mic individual. Para comparar antes/depois isso não
  atrapalha, porque o erro é o mesmo nas duas medições. Nunca apresente essa
  curva como se fosse a calibração real dele.

---

## 4. Os fatos estabelecidos — não rediscutir

Estes números sobreviveram a quatro medições e duas posições de microfone.
São a base de tudo.

### 4.1 O achado principal

> **63 Hz decai em 1,48 a 1,84 s** (média ≈ **1,66 s**), enquanto **tudo acima de
> 250 Hz decai em 0,24 a 0,47 s.**

É um problema de **uma banda só**. Não há problema de médio nem de agudo nesta
sala. Qualquer proposta que gaste dinheiro fora da região de 60 Hz está errada.

### 4.2 Não existe assimetria entre os monitores

Com o microfone na posição correta, L e R batem dentro de **2,8 dB**. A
diferença de 15 dB que apareceu numa medição era **erro de posição do
microfone**, não a sala e não o monitor.

### 4.3 O que é e o que não é confiável nesta sala

| | |
|---|---|
| **Confiável para decidir** | RT60 por banda, e o waterfall como leitura visual. Decaimento é propriedade da sala — mover o mic 20 cm muda pouco. |
| **NÃO confiável para decidir** | Resposta em frequência abaixo de 300 Hz. Mover o mic 20 cm muda **até 15 dB**. |

### 4.4 Etapa 1, concluída

Dois bass traps de lã de PET de 1,20 m foram construídos e instalados, um em
cada parede lateral. A medição acima é *com* eles no lugar.

---

## 5. Os erros já cometidos — não repetir

Esta seção existe para você não gastar o tempo dele de novo.

1. **Teoria de SBIR / porta, inventada.** Foi construída uma explicação
   elaborada para um buraco de 15 dB no canal esquerdo, envolvendo reflexão de
   limite e o vão da porta. Errada.
2. **Teoria de "monitor esquerdo quebrado".** Também errada. Chegou-se a
   recomendar que ele parasse de construir.
3. **A causa real foi encontrada por ele:** *"o mic estava na posição errada"*.
   Com o mic no lugar certo, os canais bateram.
4. **Erro de proporção na prancha de montagem:** foi escrito que a lã preenchia
   "dois terços" da câmara, contradizendo a própria cota (50 mm de 150 mm = um
   terço). Corrigido.
5. **Erro sobre a densidade da lã:** foi dito que a lã da membrana "não precisa
   ser boa". Está errado. Lã leve demais sub-amortece e deixa um pico estreito
   que pode passar ao lado de 62,4 e 64,7 Hz. **Use 30–50 kg/m³.** A lã de
   7 kg/m³ que ele já tem em casa **não serve** para a membrana.
6. **Promessa de baixar arquivos do Google Drive.** O proxy bloqueia. Não prometa.

---

## 6. Estado físico da sala em 20/09/2026

Das fotos que ele mandou:

- **Sete painéis pretos planos** nas paredes (em agosto eram dois) mais um
  **difusor de madeira** tipo skyline.
- Painéis **colados rente ao reboco, sem espaçamento** — funcionam bem só acima
  de ~1.715 Hz. Com 5 cm de sarrafo atrás iriam para 858 Hz; com 10 cm, 572 Hz.
  Foi recomendado afastar antes de medir. Confirme com ele se afastou e quanto.
- **Uma TV grande entre os dois monitores**, que ficam à frente do plano da tela.
  Reflexão dura no médio-agudo.
- **Assimetria física:** o monitor da direita está encostado no batente da porta
  e no canto; o da esquerda está mais aberto, e os suportes parecem ter alturas
  diferentes. Foi pedido que ele conferisse tweeter→parede lateral,
  tweeter→parede frontal e tweeter→orelha, batendo dentro de 2 cm.
- Mesa em L, tampo de madeira, piso de cerâmica com tapete, janela com cortina
  na parede do fundo.
- **As membranas ainda NÃO foram construídas.**

---

## 7. O que falta construir — as quatro membranas

Projeto fechado, prancha de montagem e renders 3D já entregues.

### Especificação

| | |
|---|---|
| Quantidade | **4 caixas iguais** |
| Sintonia | **63 Hz** |
| Face | MDF **8 mm** → 6,0 kg/m² |
| Câmara de ar | **150 mm** |
| Fundo | compensado **10 mm** |
| Moldura | tábua **2,5 × 15 cm**, cortada a 90°, sem ingletes |
| Lã | 50 mm de PET **30–50 kg/m³**, colada **só no fundo**, 100 mm de ar livre até a face |
| Tamanho de cada | 0,72 × 1,20 m → 3,46 m² de face no total |

Fórmula: `f = 60 / √(m · d)` com `m` em kg/m² e `d` em metros.
`60 / √(6,0 × 0,15) = 63,2 Hz` ✓

**Validade do modelo de massa-mola:** a face precisa se comportar como massa
limp, não como placa. O `f11` de flexão da chapa tem de ficar bem abaixo da
metade da sintonia. Com MDF de 8 mm o `f11` fica em ~22 Hz — ok. Com 12 mm sobe
para ~33 Hz, já perto demais. **Não troque a espessura sem refazer essa conta.**

### Montagem — pontos críticos

1. Cortar tudo antes de montar.
2. Moldura no chão plano, 2 parafusos por canto, conferir esquadro pelas diagonais.
3. Fundo rígido por cima da moldura, parafuso a cada 12 cm. É ele que dá a rigidez.
4. **Selar por dentro**, cordão de silicone neutro em todas as juntas internas.
   Uma fresta e a mola de ar vaza.
5. Lã de 50 mm colada **só no fundo**. Não pode encostar na face.
6. Face de MDF colada e parafusada a cada 10 cm, silicone na junta antes de fechar.
7. Acabamento: pintura fosca ou tecido fino esticado. **Nada de espuma ou lã na
   frente da face** — trava a chapa e mata a membrana.
8. **Encostada na parede**, no chão ou pendurada rente. Nunca no meio do vão:
   membrana trabalha sobre pressão, e pressão é máxima na parede.

### Previsão do efeito (Sabine, com α estimado)

| α assumido | A total | RT previsto em 63 Hz |
|---|---|---|
| 0,4 | 4,00 | 1,09 s |
| 0,5 | 4,34 | 1,00 s |
| **0,6** | **4,69** | **0,93 s** |
| 0,7 | 5,03 | 0,86 s |
| 0,8 | 5,38 | 0,81 s |

Partindo de `A = 0,161 · 26,96 / 1,66 = 2,62 m² sabine`.

**Diga sempre que isto é estimativa.** Em 63 Hz a sala é modal (Schroeder 496 Hz)
e Sabine é guia de ordem de grandeza, não promessa.

### Régua de decisão depois de instalar

| T20 medido em 63 Hz | leitura |
|---|---|
| até 1,05 s | funcionou dentro da previsão |
| 1,05 a 1,30 s | funcionou em parte |
| acima de 1,30 s | algo está errado na construção — ver checklist abaixo |

### Se o 63 Hz não cair, em ordem de probabilidade

1. **Caixa vazou.** Teste: empurrar a face no centro com a palma. Tem de ceder
   pouco e voltar rápido. Mole e lenta = vazou.
2. **Lã encostou na face.** Trava a chapa.
3. **Caixa não está encostada na parede.**
4. **Massa errada.** Se o MDF não for exatamente 8 mm, pesar uma sobra, calcular
   o kg/m² real e refazer `f = 60/√(m·d)`.
5. **Área de menos.** Quatro podem não bastar. Isso é dimensionamento, não erro.

---

## 8. O protocolo da medição 2

Página completa publicada: https://claude.ai/artifact/MRDJLwbC7cuGupWi66pUCV

### Sentido desta medição

Como agora há sete painéis e não dois, **esta é uma medição intermediária**, e a
pergunta que ela responde é de orçamento: *as quatro membranas ainda precisam
ser quatro?* Se o 63 Hz já caiu com os painéis novos, talvez três cheguem lá.

Ela também vira o **novo "antes"** para comparar as membranas depois.

### Antes de ligar

- Cabo XLR do mic sempre na **entrada 1** da MiniFuse.
- **Phantom +48 V ligado.**
- Monitores no volume de trabalho, knob marcado com fita.
- **SoundID Reference DESLIGADO**, e nenhum EQ de sistema ativo. Saída do sistema
  na MiniFuse, não num dispositivo virtual.
- macOS: Ajustes → Privacidade e Segurança → Microfone → REW autorizado.
  (Se estiver negado, o REW abre normal e simplesmente não escuta.)
- Posição do mic **marcada com fita crepe no chão**, cápsula na altura da orelha
  dele sentado, apontada para o ponto entre os monitores. Cadeiras fora do
  triângulo. Porta e janela fechadas, ar-condicionado e ventilador desligados.

### Preferências do REW (uma vez só)

```
Soundcard   Output Device .... MiniFuse 2
            Input Device  .... MiniFuse 2
            Input         .... In 1
            Sample rate   .... 48000
Mic/Meter   Cal file      .... ECM8000_generico.cal
```

### Ruído de fundo, antes de qualquer varredura

Aba **RTA** rodando, sala em silêncio. Anotar o piso em dBFS, principalmente
entre 50 e 100 Hz, e tirar print.

**Histórico:** em agosto o piso estava em **−45 dBFS** com o pico em **−2,3**,
ou seja **8,5 dB de SNR** — impossível medir decaimento. A causa era ruído
entrando antes da varredura; **trocar o cabo XLR levou o piso para −70 dBFS**.
Se o piso voltar a subir, procure zumbido: cabo, cabo junto da fonte, ou laptop
na tomada (testar só na bateria).

### Janela de medição

| campo | valor |
|---|---|
| Method | Sweep |
| Start / End | 15 Hz / 20 kHz |
| Length | 512 k |
| Level | −12 dBFS |
| Output | Left / Right / L+R (é aqui que se escolhe o monitor) |
| Use acoustic timing reference | **desmarcado** |
| Repetitions | **4** |

**Armadilha do REW:** `Repetitions` e `Use acoustic timing reference` são
**mutuamente exclusivos** — com a referência marcada, o campo de repetições fica
cinza. Para medir decaimento a referência não serve para nada, e 4 repetições
rendem **+6 dB de SNR** (`10·log₁₀4`). Desmarque a referência.

`Check Levels` até o pico ficar entre **−12 e −6 dBFS**. Nunca −2. Se estourar,
abaixar o **Level do REW**, não o ganho do mic. Fotografar a posição do knob.

### A sequência — quatro varreduras, sem tocar em nada entre elas

1. `Output: Left` → nome `AAAA-MM-DD L`
2. `Output: Right` → `AAAA-MM-DD R`
3. `Output: L+R` → `AAAA-MM-DD LR`
4. `Output: Left` de novo → `AAAA-MM-DD L bis` ← **teste de sanidade**

Na aba **Overlays**, sobrepor as duas medições de L: abaixo de 300 Hz têm de
coincidir dentro de **1 dB**, e o RT de 63 Hz dentro de **0,05 s**. Se não
coincidirem, algo está se mexendo e a medição inteira é lixo.

Salvar com `File → Save All Measurements` → um único `.mdat`.

### Alvo de qualidade

**SNR ≥ 40 dB.** Abaixo de 30 dB o RT60 em 63 Hz é chute. Medir à noite, porque
63 Hz é justamente onde a rua entra.

---

## 9. Como ler e analisar os arquivos `.mdat`

O `.mdat` do REW é **serialização Java**. Não existe biblioteca pronta; o leitor
abaixo foi escrito para este projeto e funciona. Está versionado no repositório
como `home-studio/mdat.py`.

### 9.1 O leitor

```python
#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Leitor de arquivos .mdat do REW (serializacao Java)."""
import struct, sys, re
import numpy as np

def strings(b, minlen=3):
    """Strings UTF do fluxo Java: 0x74 + uint16 tamanho + bytes"""
    out=[]; i=0; n=len(b)
    while True:
        i=b.find(b'\x74', i)
        if i<0 or i+3>n: break
        ln=struct.unpack_from('>H', b, i+1)[0]
        if minlen<=ln<=300 and i+3+ln<=n:
            try:
                s=b[i+3:i+3+ln].decode('utf-8')
                if s.isprintable(): out.append((i,s))
            except UnicodeDecodeError: pass
        i+=1
    return out

def arrays(b, minlen=256):
    """Arrays de float: TC_ARRAY(0x75) + TC_REFERENCE(0x71) + handle + len"""
    out=[]; i=0; n=len(b)
    while True:
        i=b.find(b'\x75\x71', i)
        if i<0 or i+10>n: break
        ln=struct.unpack_from('>i', b, i+6)[0]
        if minlen<=ln<=(n-i-10)//4:
            a=np.frombuffer(b, dtype='>f4', count=ln, offset=i+10)
            if np.isfinite(a).all() and np.abs(a).max()<1e12:
                out.append((i,ln,a.astype(np.float64)))
                i+=10+4*ln; continue
        i+=2
    return out
```

### 9.2 Como identificar o que é o quê

A 48 kHz, com varredura de 512 k:

- **Curvas de SPL**: arrays de **54.532 pontos**, passo de frequência
  `48000/131072 = 0,3662 Hz`. Um vetor de SPL típico tem mínimo entre 20 e 48 dB
  e máximo entre 88 e 112 dB — use isso para separá-los dos vetores de fase.
- **Respostas ao impulso**: arrays de **131.072 pontos**.
- Os nomes das medições aparecem como strings UTF no fluxo; use `strings()` e
  procure por data, `L`, `R`, `dBFS`, `sweep`.

O offset de cada array (`out[0]`) serve para amarrar IR e SPL à mesma medição:
elas ficam próximas no arquivo. Na prática, ordene por offset e case cada IR com
a curva de SPL que vem logo depois dela.

### 9.3 O cálculo de RT60 — use este método

Integração de Schroeder com filtros **Butterworth `sosfiltfilt`** e truncamento
no piso de ruído. **Não use máscara em FFT** (brick-wall): ela toca o filtro e
devolve RT absurdo. Isso já foi testado e falhou.

```python
import numpy as np
from scipy.signal import butter, sosfiltfilt

def rt(ir, f0, f1, fs=48000.0):
    """T20 por integracao de Schroeder.
    Devolve (T20, r2) ou None quando nao ha decaimento utilizavel.
    r2 abaixo de 0,97 = nao confie no numero."""
    ir = np.asarray(ir, float)
    p  = int(np.argmax(np.abs(ir)))
    x  = ir[max(0, p - int(.005*fs)):]                   # 5 ms antes do pico
    sos = butter(4, [f0/(fs/2), min(f1/(fs/2), .99)], btype='band', output='sos')
    e  = sosfiltfilt(sos, x)**2                          # energia filtrada
    nf = e[int(.90*len(e)):].mean()                      # piso de ruido
    acima = np.where(e > nf*10)[0]                       # trunca 10 dB acima do piso
    if len(acima) < int(.02*fs): return None
    s = np.cumsum(e[:acima[-1]][::-1])[::-1]             # integracao reversa
    L = 10*np.log10(s/s[0] + 1e-300)                     # curva de Schroeder
    t = np.arange(len(L))/fs
    m = (L <= -5) & (L >= -25)                           # janela do T20
    if m.sum() < int(.02*fs): return None
    a, b = np.polyfit(t[m], L[m], 1)
    r2 = 1 - ((L[m] - (a*t[m] + b)).var() / L[m].var())
    return -60.0/a, r2

# bandas de OITAVA -- ver 9.4 sobre por que nao usar terco de oitava em 63 Hz
BANDS = [(45,90,"63"), (90,180,"125"), (180,355,"250"), (355,710,"500"),
         (710,1420,"1k"), (1420,2840,"2k"), (2840,5680,"4k")]
```

**Por que T20 e não T30:** T30 precisa de ~35 dB de decaimento limpo acima do
ruído. Em 63 Hz, numa sala de 27 m³, quase nunca existe — e quando não existe, o
ajuste cai em cima do ruído e devolve um número bonito e falso. T20 precisa de 25.
Se `rt()` devolver `None`, **diga que não deu para medir**, não chute.

**Sempre reporte o `r2` junto com o número.** Abaixo de 0,97 a curva de
Schroeder não é uma reta no trecho ajustado e o T20 não significa nada.

### 9.4 O limite estatístico em 63 Hz — leia antes de interpretar qualquer número

Este código foi validado por Monte Carlo com decaimentos sintéticos de valor
conhecido (200 realizações, alvo de 1,66 s):

| banda | largura × T | mediana medida | desvio | p10 … p90 |
|---|---|---|---|---|
| 63 Hz, **oitava** (45–90) | BT = 75 | 1,638 s | **±14,9 %** | 1,36 … 1,92 |
| 63 Hz, **terço de oitava** (56–71) | BT = 25 | 1,611 s | **±23,8 %** | 1,16 … 2,06 |
| 500 Hz, oitava | BT = 589 | 1,667 s | ±3,3 % | 1,60 … 1,74 |

Num decaimento sem filtro nenhum o código devolve o valor exato (0,500 · 0,996 ·
1,657 para alvos de 0,5 · 1,0 · 1,66). **O método é correto e não é enviesado** —
a mediana acerta. O que existe é **dispersão estatística**, e ela é grande em
63 Hz porque a banda tem só 45 Hz de largura. A ISO 3382 pede BT > 16 para a
estimativa valer; em 63 Hz a gente está em 75, que passa, mas sem folga.

**Três consequências práticas, todas importantes:**

1. **Use banda de OITAVA em 63 Hz, não terço de oitava.** O terço quase dobra a
   dispersão (24 % contra 15 %) sem trazer informação que decida nada. Terço de
   oitava serve para olhar o formato da curva; a decisão sai da oitava.
2. **Uma única medição de 63 Hz carrega ±15 %.** Os quatro valores históricos de
   1,48 a 1,84 s são exatamente essa dispersão em volta de 1,66 — **não são
   medições ruins, são a estatística da banda.** Não trate a variação entre eles
   como se fosse informação.
3. **Uma mudança menor que ~15 % não é resultado.** Para afirmar que as membranas
   funcionaram, o T20 precisa cair bem mais que isso. A previsão é de 1,66 para
   ~0,93 s, uma queda de **44 %** — confortavelmente fora do ruído. Mas se cair só
   para 1,45 s (−13 %), **diga que não deu para provar nada** em vez de comemorar.
   Média de L, R e LR reduz a dispersão para ~8,6 % (√3).

### 9.5 Auto-teste antes de confiar no pipeline

Rode isto antes de analisar os arquivos dele. Se não reproduzir a tabela acima,
alguma coisa está errada no seu ambiente:

```python
import numpy as np
fs = 48000.0
rng = np.random.default_rng(7)
t = np.arange(int(6*fs))/fs
alvo = 1.66
out = []
for _ in range(200):
    ir = rng.standard_normal(len(t)) * np.exp(-6.907/alvo * t)
    r = rt(ir, 45, 90)
    if r: out.append(r[0])
o = np.array(out)
print(f"mediana {np.median(o):.3f} s  (alvo {alvo})   desvio {o.std()/np.median(o)*100:.1f}%")
# esperado: mediana ~1,64 s, desvio ~15%
```

### 9.4 Normalização para comparar curvas

Para sobrepor L e R (ou antes e depois), alinhe pela média de **400–4000 Hz**,
que é a região estável:

```python
ref = (f >= 400) & (f < 4000)
CL = SL - SL[ref].mean()
CR = SR - SR[ref].mean()
```

E suavize em 1/6 de oitava antes de mostrar, senão o gráfico vira ruído visual.

---

## 10. Como apresentar o resultado para ele

- Compare o **T20 da banda de oitava de 63 Hz** com **1,66 s**, e diga em quantos
  por cento caiu — lembrando que abaixo de ~15 % a queda não é distinguível da
  dispersão da própria banda.
- Mostre a tabela de RT60 por banda de oitava, L e R lado a lado.
- Diga explicitamente **quanto SNR a medição teve** e se o número de 63 Hz é
  confiável ou não.
- Se ele afastou os painéis da parede, tente separar o efeito disso.
- Refaça a conta das membranas com o RT novo: `A = 0,161·V/RT`, e veja quantas
  caixas ainda faltam para chegar em ~0,9 s.
- **Não venda otimismo.** Se caiu pouco, diga que caiu pouco.

### Paleta, se for gerar gráfico

O projeto usa `#1670b0` (azul) e `#c2703a` (laranja) para as séries. A dupla
verde/laranja **foi reprovada** em teste de daltonismo protan (ΔE 5,3) e não deve
voltar.

---

## 11. Os outros projetos desta conversa

Não são objeto desta sessão, mas existem e ele pode citar:

| projeto | estado | link |
|---|---|---|
| **Quarto de casal** (dele e da Cíntia) — 3,80 × 3,80, ripado com TV, guarda-roupa de 3 m, penteadeira suspensa | conceito fechado, mockups gerados | https://claude.ai/code/artifact/dc0d08a0-6849-4bd5-b62c-f947e4ad99da |
| **Loja Axozee** — sistema de som para eventos com DJ, 39 m² | projeto entregue | https://claude.ai/artifact/38uPrStpKczCjHpC9J1fg5 |
| **Reels dos bass traps** — edição de vídeo de construção para o Instagram | pausado. Falta confirmar o crédito da faixa: o vídeo diz *thinking right · SNOE*, ele disse *Stomp · EP By the River · Elevation* | — |
| **Fone de ouvido** na casa dos R$ 3.000 | pausado por escolha dele. Olhou HD 490 Pro e HD 650 | — |

---

## 12. Material do projeto

Repositório: `destaquebones-bit/claude-code-nano-banana`, branch
`claude/home-studio-structure-analysis-0wfzwn`, pasta `home-studio/`.

| arquivo | o que é |
|---|---|
| `mdat.py` | leitor de `.mdat` do REW |
| `resultado.py` → `resultado.png` | prancha de RT60 da etapa 1 |
| `montagem-membrana.py` → `.png` | passo a passo de montagem da membrana |
| `membrana.py` → `.png` | prancha de projeto do absorvedor |
| `medicao-2.html` | protocolo da medição 2, com o REW clique a clique |
| `manual-rew.html` | manual geral do REW, da primeira medição |
| `ECM8000_generico.cal` | curva de calibração genérica do mic |
| `mockups/` | renders 3D da membrana e montagens sobre foto real |
| `fotos/` | fotos da sala (fora do versionamento) |

### Páginas publicadas

- Medição 2 — https://claude.ai/artifact/MRDJLwbC7cuGupWi66pUCV
- Sala 3 × 4,8 — https://claude.ai/code/artifact/56852314-77af-49cf-a071-86d65d416694
- Caderno de Obra — https://claude.ai/code/artifact/37f67936-c761-4d7e-a3c3-4b1e9efe1d20
- Bibliografia — https://claude.ai/code/artifact/b821551a-6f47-4def-9141-469b2348c341

---

## 13. O próximo passo, em uma frase

**Ele vai rodar a medição 2 e mandar um `.mdat` com quatro varreduras, um print
do ruído de fundo e uma foto da marca de fita no chão. Leia o `.mdat` com o
código da seção 9, calcule T20 por banda, compare 63 Hz com 1,66 s, diga se o
número é confiável, e refaça o dimensionamento das membranas com o valor real.**
