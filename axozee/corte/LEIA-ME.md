# Plano de Corte — AXOZEE

`AXOZEE-Plano-de-Corte-v1.xlsx` é a planilha que vai para o corte.
`gerar-planilha.py` reconstrói o arquivo do zero (`python3.13 gerar-planilha.py`) — útil
para gerar a cápsula 02 sem refazer formatação à mão.

## Abas

| Aba | Para quê |
|---|---|
| **PAINEL** | Acompanhamento. Tudo calculado: KPIs, tabelas por malha/modelagem/tamanho, rendimento e três gráficos. |
| **PLANO DE CORTE** | A folha de trabalho do corte. 16 grades (2 malhas × 2 modelagens × 4 tamanhos). |
| **MALHAS** | Estoque em kg, rendimento previsto e real, saldo. |
| **MODELAGENS** | Regras de gola — alimentam a coluna "Gola no corte" do plano. |
| **COMO USAR** | Legenda de cores, status e fluxo do dia. |

## O que é automático

- **Status** por grade: SEM PLANO · PENDENTE · EM CORTE · CORTADO · EXCEDENTE,
  derivado da quantidade cortada contra a planejada.
- **Gola no corte** por linha, via modelagem (Slim corta gola da própria malha; Over não corta,
  ribana canelada na costura).
- **Kg previstos** por grade = quantidade ÷ rendimento da malha.
- **Rendimento real** = peças cortadas ÷ kg realmente consumidos, com desvio contra o previsto.
- Listas suspensas em malha, modelagem, tamanho e responsável; bloqueio de quantidade negativa.
- Barras de progresso, cores por status, filtro no cabeçalho, painéis congelados.
- Impressão já configurada: paisagem, uma página de largura, cabeçalho repetido, rodapé com data.

## Dados da v1 (informados por Jhonatan em 13/10/2026)

- Algodão Pima Preta — 12 kg, rendimento 5,0 pç/kg → capacidade 60 peças.
- Peruana Off White — 18 kg, rendimento 4,5 pç/kg → capacidade 81 peças.
- Plano: 140 peças (60 na Pima, 80 na Peruana). Sobram 0,22 kg de Peruana — cabe 1 peça a mais.
- Golas a cortar: 60 (todas as Slim).
