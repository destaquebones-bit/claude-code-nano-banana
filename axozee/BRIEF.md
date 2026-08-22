# AXOZEE — Camisetas | Brief de Projeto

Documento vivo do desenvolvimento dos modelos de camiseta da marca **AXOZEE**.
Atualizado a cada rodada de trabalho.

## Status

| Item | Situação |
|---|---|
| Referências (camisetas Totanka, no Google Drive) | ✅ **Lidas** — 13 peças analisadas, DNA em `REFERENCIAS-TOTANKA.md` |
| Identidade visual da AXOZEE | ✅ **Brand Book V1 recebido** (22/08) — resumo em `MARCA.md`, PDF em `marca/` |
| Posicionamento | Definido: mistura dos três eixos (música eletrônica, urbano/streetwear, lifestyle) |
| Cápsula 01 "ASFALTO × SELVA" | ✅ **6 peças** em `COLECAO-01.md`, mockups em `artes/`, lookbook em `lookbook-capsula-01.html` |
| Rodada "Night Shift" (pré-brand book) | ⛔ Descartada — motivo em `artes/_descartadas/LEIA-ME.md` |


## Contexto da marca

- **AXOZEE** — streetwear unissex de Jhonatan Mandato, sócio da Destaque Bonés e Confecções.
  Conceito **ASFALTO × SELVA**, "o aço que se move" (ver `MARCA.md`). Domínio `axozee.com.br`
  registrado em 13/08/2026 (Registro.br → Cloudflare → VPS KingHost); ainda não resolve DNS.
- **Posicionamento definido pelo Jhonatan:** "um pouco dos três" — a linha deve
  transitar entre:
  1. **Música eletrônica** — universo tech house / rave / festival / cultura de pista;
  2. **Urbano / streetwear** — oversized, grafismo, lettering (referência declarada: Totanka);
  3. **Lifestyle** — peças mais limpas, foco em logo e minimalismo.
  Na prática isso vira uma **cápsula com três famílias de estampa** dentro da mesma
  identidade, não três marcas separadas.

## Referência declarada: Totanka

Não foi possível abrir o site da Totanka deste ambiente (`totanka.com.br` bloqueado
pelo proxy de rede da sessão) nem o Drive. O que foi levantado por busca pública,
a **confirmar contra as peças reais do Drive**:

- Modelagem **oversized** (a marca trabalha grades tipo "Oversized 2" e "Oversized 5").
- Malhas: **molecotton** e **suedine** — caimento amplo, toque macio, leve estrutura.
- Estampa **frente e costas**, técnicas: **silk zero toque** (acabamento leve, sem
  enrijecer o tecido) e **plastisol em alto relevo**.
- Paleta recorrente: **offwhite** e **preto**.
- **Etiquetas próprias** como elemento de marca (interna e/ou externa).
- Coleções sazonais + peças temáticas (ex.: linha Copa 2026).

Fontes: totanka.com.br (loja), linha oversized, exemplo "Camiseta Oversized 2 Better Days Offwhite".

## Especificação de produção (baseline a validar)

- Modelagem: oversized, gola careca com ribana reforçada.
- Malha: molecotton (peso maior, caimento estruturado) ou suedine (mais leve).
- Estampa: silk zero toque para artes grandes/chapadas; plastisol alto relevo para
  logo e detalhes que pedem textura.
- Posições: peito (localizada ou centralizada), costas (arte grande), manga (opcional),
  nuca (logo pequeno).
- Cores base da peça: offwhite e preto como núcleo; cor sazonal por cápsula.

## Pipeline de trabalho

1. **Referências** → analisar as peças da Totanka no Drive + identidade AXOZEE.
2. **Direção** → fechar as três famílias de estampa e a régua de marca (onde entra
   logo, onde entra grafismo, onde entra tipografia).
3. **Artes** → gerar as estampas (geração de imagem via artlist, disponível nesta sessão).
4. **Mockups** → aplicar as artes em mockup de camiseta oversized, frente e costas.
5. **Arquivo editável** → montar no Canva (conector disponível) para ajuste fino e
   envio à confecção.

Tudo versionado aqui: `axozee/referencias/`, `axozee/artes/`, `axozee/mockups/`.

## Bloqueios atuais

1. **Garra da peça S-01** — o mockup entregou um rasgo largo de fera; o brand device são
   quatro lâminas finas, curvas e escalonadas. Regerar.
2. **Escolha das peças** — quais das seis vão para arte final e quais entram na rodada 2.
3. **Tipografia definitiva** — o manual ainda usa faces de sistema; decidir entre a rota
   didone e a glífica flarada antes de licenciar e converter o wordmark em curvas.

~~Identidade~~ e ~~geração de imagem~~ — resolvidos em 22/08/2026.

O Google Drive já está conectado e funcionando (resolvido em 22/08/2026).
