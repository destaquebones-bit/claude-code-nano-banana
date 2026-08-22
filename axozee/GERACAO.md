# Geração das artes — estado das ferramentas

O que falta para sair do esquema e chegar na arte final imprimível.

## Caminhos testados nesta sessão

| Ferramenta | Estado | Observação |
|---|---|---|
| **artlist** (gerador de imagem) | ❌ sem créditos | A conta retornou `insufficient_credits`. Recarga: https://artlist.io/checkout/suite |
| **Gemini / Nano Banana** (skill `nano-banana`) | ⚠️ falta chave | A API do Google **está acessível** desta sessão (responde 403 por falta de identidade, não por bloqueio de rede). Basta uma `GEMINI_API_KEY` — chave gratuita em https://aistudio.google.com/apikey |
| **Canva** | ✅ disponível | Serve para montar o arquivo editável e exportar PDF/PNG, não para gerar a ilustração |
| **Google Drive** | ✅ conectado | Referências da Totanka lidas com sucesso |

## Recomendação

Usar o **Nano Banana Pro** (`--resolution 2K`) via skill `nano-banana`: é o modelo com melhor
renderização de texto, e as estampas desta cápsula são quase todas tipográficas. Fluxo:

```
GEMINI_API_KEY="<chave>" python3 ~/.claude/skills/synced/nano-banana/scripts/genimage.py \
  --prompt "<prompt da peça, em COLECAO-01.md>" \
  --aspect-ratio 4:5 --resolution 2K \
  --output axozee/artes/<codigo>.png
```

Ordem sugerida de geração:
1. **B-04 Santos da Madrugada** — gravura dos anjos em halftone (a peça que mais depende de arte ilustrada).
2. **B-03 World Tour** — escudos rodoviários desgastados.
3. **A-01 Axozee Stadium** — textura de silk craquelado sobre a tipografia.
4. Mockups fotográficos das seis peças, no mesmo enquadramento de catálogo das fotos da Totanka.

## Depois da geração

- Arte final e mockups vão para `axozee/artes/` e `axozee/mockups/`.
- Montagem do arquivo editável no Canva, exportado em PDF para a confecção.
- Separação de cores por peça (a confecção precisa saber quantas cores tem cada silk).
