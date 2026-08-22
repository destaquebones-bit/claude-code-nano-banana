# AXOZEE — Camisetas | Brief de Projeto

Documento vivo do desenvolvimento dos modelos de camiseta da marca **AXOZEE**.
Atualizado a cada rodada de trabalho.

## Status

| Item | Situação |
|---|---|
| Referências (camisetas Totanka, no Google Drive) | ✅ **Lidas** — 13 peças analisadas, DNA em `REFERENCIAS-TOTANKA.md` |
| Identidade visual da AXOZEE (logo, paleta, tipografia) | ❌ **Não está no Drive** — só as pastas TOTANKA e MKT DIGITAL. Precisa ser enviada |
| Posicionamento | Definido: mistura dos três eixos (música eletrônica, urbano/streetwear, lifestyle) |
| Rodada 1 — conceito | ✅ **Entregue** — 6 modelos em `COLECAO-01.md` + lookbook `lookbook-capsula-01.html` |
| Rodada 1 — arte final | ✅ **6 mockups gerados em 2K** em `axozee/artes/` (Nano Banana Pro) |

## Contexto da marca

- **AXOZEE** — marca de Jhonatan Mandato (também produtor de tech house sob o nome
  *Spectral Sun*). Domínio `axozee.com.br` registrado em 13/08/2026 (Registro.br →
  Cloudflare → VPS KingHost). O site ainda não resolve DNS externamente.
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

1. **Arquivos de identidade da AXOZEE** — procurados em 22/08/2026 no Google Drive
   (só as pastas TOTANKA e MKT DIGITAL), no Canva (nenhum design com "axozee"), nos anexos
   do Gmail e no sistema de arquivos desta sessão. **Não foram encontrados.** Esta sessão
   roda em container remoto na nuvem, não na máquina do Jhonatan — o único jeito de trazer
   os arquivos é subir no Drive, anexar no chat, ou rodar o Claude Code localmente.
2. ~~Geração de imagem~~ — resolvido: chave do Gemini fornecida, artes geradas com
   Nano Banana Pro em 2K. Comandos em `GERACAO.md`.

O Google Drive já está conectado e funcionando (resolvido em 22/08/2026).
