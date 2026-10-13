#!/usr/bin/env python3.13
# -*- coding: utf-8 -*-
"""Gera a planilha de plano de corte da AXOZEE."""

from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
from openpyxl.utils import get_column_letter
from openpyxl.worksheet.datavalidation import DataValidation
from openpyxl.formatting.rule import CellIsRule, DataBarRule, FormulaRule
from openpyxl.chart import BarChart, DoughnutChart, Reference
from openpyxl.comments import Comment

OUT = "/home/user/claude-code-nano-banana/axozee/corte/AXOZEE-Plano-de-Corte-v1.xlsx"

# ---------- paleta da marca ----------
ASFALTO   = "11110F"
GRAFITE   = "26261F"
TERRACOTA = "C45D36"
OLIVA     = "555C3A"
AREIA     = "EEE7D6"
AREIA_CL  = "F7F3EA"
INPUT_BG  = "FFF0C9"   # célula que o usuário preenche
CALC_BG   = "FFFFFF"
BORDA     = "C9C2B2"

VERDE  = "2E7D32"; VERDE_BG  = "D9EAD3"
AMBAR  = "A86B00"; AMBAR_BG  = "FFF2CC"
CINZA  = "7A7568"; CINZA_BG  = "EFEDE7"
VERM   = "A32D1E"; VERM_BG   = "F9D9D3"

F = "Arial"
thin = Side(style="thin", color=BORDA)
BOX = Border(left=thin, right=thin, top=thin, bottom=thin)

def style(ws, ref, *, bold=False, size=10, color="000000", bg=None, align="left",
          wrap=False, fmt=None, border=True, italic=False):
    for row in ws[ref]:
        for c in row:
            c.font = Font(name=F, bold=bold, size=size, color=color, italic=italic)
            if bg:
                c.fill = PatternFill("solid", fgColor=bg)
            c.alignment = Alignment(horizontal=align, vertical="center", wrap_text=wrap)
            if border:
                c.border = BOX
            if fmt:
                c.number_format = fmt

def titulo(ws, cell, texto, span, sub=None, sub_span=None):
    """Faixa de título no topo da aba."""
    ws[cell] = texto
    ws.merge_cells(f"{cell}:{span}")
    style(ws, f"{cell}:{span}", bold=True, size=16, color=AREIA, bg=ASFALTO,
          align="left", border=False)
    ws.row_dimensions[int(cell[1:])].height = 34
    if sub:
        r = int(cell[1:]) + 1
        col = cell[0]
        ws[f"{col}{r}"] = sub
        fim = sub_span or span[0]
        ws.merge_cells(f"{col}{r}:{fim}{r}")
        style(ws, f"{col}{r}:{fim}{r}", size=9, color=AREIA, bg=GRAFITE,
              align="left", border=False)
        ws.row_dimensions[r].height = 18

def cabecalho(ws, row, col_ini, headers, widths):
    for i, (h, w) in enumerate(zip(headers, widths)):
        col = get_column_letter(col_ini + i)
        ws[f"{col}{row}"] = h
        ws.column_dimensions[col].width = w
    ref = f"{get_column_letter(col_ini)}{row}:{get_column_letter(col_ini+len(headers)-1)}{row}"
    style(ws, ref, bold=True, size=9, color=AREIA, bg=OLIVA, align="center", wrap=True)
    ws.row_dimensions[row].height = 32

wb = Workbook()

# =====================================================================
# ABA 1 — PAINEL  (criada agora, preenchida no fim, para ficar primeira)
# =====================================================================
painel = wb.active
painel.title = "PAINEL"

plano = wb.create_sheet("PLANO DE CORTE")
malhas = wb.create_sheet("MALHAS")
model = wb.create_sheet("MODELAGENS")
ajuda = wb.create_sheet("COMO USAR")

# =====================================================================
# ABA MALHAS
# =====================================================================
titulo(malhas, "A1", "MALHAS  ·  ESTOQUE E RENDIMENTO", "N1",
       "Linhas em amarelo são preenchidas por você. O resto é calculado.")

MAL_H = ["Código", "Malha", "Cor", "Kg disponível", "Rendimento previsto (pç/kg)",
         "Capacidade prevista (pç)", "Peças planejadas", "Kg previsto p/ o plano",
         "Peças cortadas", "Kg real consumido", "Rendimento real (pç/kg)",
         "Desvio vs previsto", "Saldo de malha (kg)", "Nome usado no plano de corte"]
MAL_W = [9, 18, 12, 12, 14, 13, 12, 13, 11, 13, 13, 12, 13, 24]
cabecalho(malhas, 4, 1, MAL_H, MAL_W)

MALHA_DADOS = [
    ("M1", "Algodão Pima", "Preta",     12, 5.0),
    ("M2", "Peruana",      "Off White", 18, 4.5),
]
r = 5
for cod, nome, cor, kg, rend in MALHA_DADOS:
    malhas[f"A{r}"] = cod
    malhas[f"B{r}"] = nome
    malhas[f"C{r}"] = cor
    malhas[f"D{r}"] = kg
    malhas[f"E{r}"] = rend
    malhas[f"F{r}"] = f"=D{r}*E{r}"
    malhas[f"N{r}"] = f"=B{r}&\" \"&C{r}"
    malhas[f"G{r}"] = f"=SUMIFS('PLANO DE CORTE'!$F$7:$F$22,'PLANO DE CORTE'!$B$7:$B$22,$N{r})"
    malhas[f"H{r}"] = f"=IFERROR(G{r}/E{r},0)"
    malhas[f"I{r}"] = f"=SUMIFS('PLANO DE CORTE'!$G$7:$G$22,'PLANO DE CORTE'!$B$7:$B$22,$N{r})"
    malhas[f"J{r}"] = None                       # input
    malhas[f"K{r}"] = f"=IFERROR(I{r}/J{r},0)"
    malhas[f"L{r}"] = f'=IF(J{r}=0,"—",IFERROR(K{r}/E{r}-1,0))'
    malhas[f"M{r}"] = f"=D{r}-IF(J{r}=0,H{r},J{r})"
    r += 1
ULT_MAL = r - 1

style(malhas, f"A5:C{ULT_MAL}", size=10, align="left")
style(malhas, f"A5:A{ULT_MAL}", bold=True, size=10, align="center", color=TERRACOTA)
style(malhas, f"D5:E{ULT_MAL}", size=10, align="center", bg=INPUT_BG, fmt="#,##0.0")
style(malhas, f"F5:I{ULT_MAL}", size=10, align="center", fmt="#,##0.0")
style(malhas, f"J5:J{ULT_MAL}", size=10, align="center", bg=INPUT_BG, fmt="#,##0.00")
style(malhas, f"K5:K{ULT_MAL}", size=10, align="center", fmt="#,##0.00", bold=True)
style(malhas, f"L5:L{ULT_MAL}", size=10, align="center", fmt="+0.0%;-0.0%;0.0%")
style(malhas, f"M5:M{ULT_MAL}", size=10, align="center", fmt="#,##0.00")
style(malhas, f"N5:N{ULT_MAL}", bold=True, size=10, align="left", color=OLIVA)
for rr in range(5, ULT_MAL + 1):
    malhas.row_dimensions[rr].height = 24

# total
tr = ULT_MAL + 1
malhas[f"A{tr}"] = "TOTAL"
malhas.merge_cells(f"A{tr}:C{tr}")
for col in "DFGHIJM":
    malhas[f"{col}{tr}"] = f"=SUM({col}5:{col}{ULT_MAL})"
malhas[f"K{tr}"] = f"=IFERROR(I{tr}/J{tr},0)"
style(malhas, f"A{tr}:N{tr}", bold=True, size=10, color=AREIA, bg=GRAFITE, align="center",
      fmt="#,##0.0")
malhas[f"A{tr}"].alignment = Alignment(horizontal="left", vertical="center")

malhas[f"J{ULT_MAL+3}"] = ("Preencha 'Kg real consumido' depois do corte: é ele que revela o "
                           "rendimento real da malha e o desvio contra o previsto.")
malhas.merge_cells(f"A{ULT_MAL+3}:N{ULT_MAL+3}")
malhas[f"A{ULT_MAL+3}"] = ("Preencha 'Kg real consumido' depois do corte — é ele que revela o "
                           "rendimento real e o desvio contra o previsto. "
                           "Kg disponível e rendimento previsto vieram de você (12 kg a 5 pç/kg; "
                           "18 kg a 4,5 pç/kg).")
style(malhas, f"A{ULT_MAL+3}:N{ULT_MAL+3}", size=9, italic=True, color=CINZA,
      bg=AREIA_CL, align="left", wrap=True, border=False)
malhas.row_dimensions[ULT_MAL+3].height = 30

malhas["L4"].comment = Comment("Rendimento real dividido pelo previsto. Positivo = a malha "
                               "rendeu mais peças por quilo do que o esperado.", "AXOZEE")
malhas.freeze_panes = "A5"

# =====================================================================
# ABA MODELAGENS
# =====================================================================
titulo(model, "A1", "MODELAGENS  ·  REGRAS DE CORTE", "G1",
       "Estas regras alimentam automaticamente a coluna 'Gola' do plano de corte.")

MOD_H = ["Modelagem", "Corta gola?", "Tipo de gola", "Instrução para o corte",
         "Peças planejadas", "Peças cortadas", "Golas a cortar"]
cabecalho(model, 4, 1, MOD_H, [18, 12, 20, 52, 13, 13, 13])

MOD_DADOS = [
    ("Axozee Slim", "SIM", "Gola da própria malha",
     "Cortar a gola na própria malha. O risco da gola já está na modelagem — usar o molde como está."),
    ("Axozee Over", "NÃO", "Ribana canelada",
     "Não cortar gola. A ribana canelada é aplicada na costura — conferir ribana separada antes de fechar o pacote."),
]
r = 5
for nome, gola, tipo, instr in MOD_DADOS:
    model[f"A{r}"] = nome
    model[f"B{r}"] = gola
    model[f"C{r}"] = tipo
    model[f"D{r}"] = instr
    model[f"E{r}"] = f"=SUMIFS('PLANO DE CORTE'!$F$7:$F$22,'PLANO DE CORTE'!$C$7:$C$22,$A{r})"
    model[f"F{r}"] = f"=SUMIFS('PLANO DE CORTE'!$G$7:$G$22,'PLANO DE CORTE'!$C$7:$C$22,$A{r})"
    model[f"G{r}"] = f'=IF($B{r}="SIM",E{r},0)'
    model.row_dimensions[r].height = 46
    r += 1
ULT_MOD = r - 1

style(model, f"A5:A{ULT_MOD}", bold=True, size=11, align="left")
style(model, f"B5:B{ULT_MOD}", bold=True, size=11, align="center")
style(model, f"C5:C{ULT_MOD}", size=10, align="left")
style(model, f"D5:D{ULT_MOD}", size=10, align="left", wrap=True)
style(model, f"E5:G{ULT_MOD}", size=11, align="center", fmt="#,##0")
model.conditional_formatting.add(f"B5:B{ULT_MOD}",
    CellIsRule(operator="equal", formula=['"SIM"'], fill=PatternFill("solid", fgColor=VERDE_BG),
               font=Font(name=F, bold=True, color=VERDE)))
model.conditional_formatting.add(f"B5:B{ULT_MOD}",
    CellIsRule(operator="equal", formula=['"NÃO"'], fill=PatternFill("solid", fgColor=AMBAR_BG),
               font=Font(name=F, bold=True, color=AMBAR)))

model[f"A{ULT_MOD+2}"] = ("Regra de ouro do pacote: toda peça Slim leva gola cortada junto; "
                          "nenhuma peça Over leva — a ribana vai para a costura.")
model.merge_cells(f"A{ULT_MOD+2}:G{ULT_MOD+2}")
style(model, f"A{ULT_MOD+2}:G{ULT_MOD+2}", size=10, bold=True, color=ASFALTO, bg=AREIA,
      align="left", wrap=True, border=False)
model.row_dimensions[ULT_MOD+2].height = 26

# =====================================================================
# ABA PLANO DE CORTE
# =====================================================================
titulo(plano, "A1", "PLANO DE CORTE  ·  CÁPSULA 01", "N1",
       "Preencha apenas as colunas amarelas. Status, saldo, gola e consumo de malha se atualizam sozinhos.",
       sub_span="K")

# faixa de resumo rápido no topo
plano["L2"] = "ATUALIZADO EM"
plano.merge_cells("L2:M2")
plano["N2"] = "=TODAY()"
style(plano, "L2:M2", bold=True, size=9, color=AREIA, bg=GRAFITE, align="right", border=False)
style(plano, "N2:N2", bold=True, size=9, color=AREIA, bg=GRAFITE, align="center",
      fmt="DD/MM/YYYY", border=False)

KPI_STRIP = [
    ("A3:B3", "PLANEJADO", "C3", "=SUM(F7:F22)", "#,##0", AREIA),
    ("D3:E3", "CORTADO",   "F3", "=SUM(G7:G22)", "#,##0", AREIA),
    ("G3:H3", "FALTA",     "I3", "=C3-F3",       "#,##0", AREIA),
    ("J3:K3", "CONCLUSÃO", "L3", "=IFERROR(SUM(G7:G22)/SUM(F7:F22),0)", "0%", TERRACOTA),
    ("M3:M3", "GOLAS",     "N3", "=MODELAGENS!G5+MODELAGENS!G6", "#,##0", AREIA),
]
for rotulo_ref, rotulo, cel, formula, fmt, bg in KPI_STRIP:
    ini = rotulo_ref.split(":")[0]
    plano[ini] = rotulo
    if rotulo_ref.split(":")[0] != rotulo_ref.split(":")[1]:
        plano.merge_cells(rotulo_ref)
    style(plano, rotulo_ref, bold=True, size=9, color=AREIA, bg=GRAFITE, align="right")
    plano[cel] = formula
    cor_txt = "FFFFFF" if bg == TERRACOTA else ASFALTO
    style(plano, f"{cel}:{cel}", bold=True, size=13, color=cor_txt, bg=bg,
          align="center", fmt=fmt)
plano.row_dimensions[3].height = 28

PL_H = ["Item", "Malha", "Modelagem", "Tamanho", "Gola no corte", "Qtd planejada",
        "Qtd cortada", "Falta", "% cortado", "Status", "Kg previstos",
        "Data do corte", "Responsável", "Observações"]
PL_W = [8, 20, 15, 9, 21, 11, 11, 8, 10, 15, 11, 12, 14, 22]
cabecalho(plano, 6, 1, PL_H, PL_W)

GRADE = [
    ("Algodão Pima Preta", "Axozee Slim", "P", 5),
    ("Algodão Pima Preta", "Axozee Slim", "M", 10),
    ("Algodão Pima Preta", "Axozee Slim", "G", 20),
    ("Algodão Pima Preta", "Axozee Slim", "GG", 15),
    ("Algodão Pima Preta", "Axozee Over", "P", 0),
    ("Algodão Pima Preta", "Axozee Over", "M", 0),
    ("Algodão Pima Preta", "Axozee Over", "G", 10),
    ("Algodão Pima Preta", "Axozee Over", "GG", 0),
    ("Peruana Off White", "Axozee Slim", "P", 0),
    ("Peruana Off White", "Axozee Slim", "M", 0),
    ("Peruana Off White", "Axozee Slim", "G", 5),
    ("Peruana Off White", "Axozee Slim", "GG", 5),
    ("Peruana Off White", "Axozee Over", "P", 10),
    ("Peruana Off White", "Axozee Over", "M", 20),
    ("Peruana Off White", "Axozee Over", "G", 20),
    ("Peruana Off White", "Axozee Over", "GG", 20),
]
r = 7
for i, (mal, mod, tam, qtd) in enumerate(GRADE, start=1):
    plano[f"A{r}"] = f"CP-{i:02d}"
    plano[f"B{r}"] = mal
    plano[f"C{r}"] = mod
    plano[f"D{r}"] = tam
    plano[f"E{r}"] = f"=IFERROR(INDEX(MODELAGENS!$C$5:$C$6,MATCH($C{r},MODELAGENS!$A$5:$A$6,0)),\"\")"
    plano[f"F{r}"] = qtd
    plano[f"G{r}"] = 0
    plano[f"H{r}"] = f"=MAX(0,F{r}-G{r})"
    plano[f"I{r}"] = f"=IFERROR(G{r}/F{r},0)"
    plano[f"J{r}"] = (f'=IF(F{r}=0,"SEM PLANO",IF(G{r}=0,"PENDENTE",'
                      f'IF(G{r}<F{r},"EM CORTE",IF(G{r}=F{r},"CORTADO","EXCEDENTE"))))')
    plano[f"K{r}"] = (f"=IFERROR(F{r}/INDEX(MALHAS!$E$5:$E$6,MATCH($B{r},MALHAS!$N$5:$N$6,0)),0)")
    r += 1
ULT = r - 1   # 22

style(plano, f"A7:A{ULT}", bold=True, size=9, align="center", color=TERRACOTA)
style(plano, f"B7:D{ULT}", size=10, align="center", bg=INPUT_BG)
style(plano, f"E7:E{ULT}", size=9, align="center", color=CINZA, italic=True)
style(plano, f"F7:F{ULT}", bold=True, size=11, align="center", bg=INPUT_BG, fmt="#,##0")
style(plano, f"G7:G{ULT}", bold=True, size=11, align="center", bg=INPUT_BG, fmt="#,##0")
style(plano, f"H7:H{ULT}", size=10, align="center", fmt="#,##0")
style(plano, f"I7:I{ULT}", size=10, align="center", fmt="0%")
style(plano, f"J7:J{ULT}", bold=True, size=9, align="center")
style(plano, f"K7:K{ULT}", size=10, align="center", fmt="#,##0.00")
style(plano, f"L7:L{ULT}", size=10, align="center", bg=INPUT_BG, fmt="DD/MM/YYYY")
style(plano, f"M7:M{ULT}", size=10, align="center", bg=INPUT_BG)
style(plano, f"N7:N{ULT}", size=9, align="left", bg=INPUT_BG, wrap=True)
for rr in range(7, ULT + 1):
    plano.row_dimensions[rr].height = 22

# linha de total
tr = ULT + 1
plano[f"A{tr}"] = "TOTAL"
plano.merge_cells(f"A{tr}:E{tr}")
plano[f"F{tr}"] = f"=SUM(F7:F{ULT})"
plano[f"G{tr}"] = f"=SUM(G7:G{ULT})"
plano[f"H{tr}"] = f"=SUM(H7:H{ULT})"
plano[f"I{tr}"] = f"=IFERROR(G{tr}/F{tr},0)"
plano[f"K{tr}"] = f"=SUM(K7:K{ULT})"
plano[f"L{tr}"] = f'=COUNTIF(J7:J{ULT},"CORTADO")&" de "&COUNTIF(J7:J{ULT},"<>SEM PLANO")&" grades ok"'
plano.merge_cells(f"L{tr}:N{tr}")
style(plano, f"A{tr}:N{tr}", bold=True, size=11, color=AREIA, bg=ASFALTO, align="center")
plano[f"A{tr}"].alignment = Alignment(horizontal="left", vertical="center")
plano[f"F{tr}"].number_format = "#,##0"
plano[f"G{tr}"].number_format = "#,##0"
plano[f"H{tr}"].number_format = "#,##0"
plano[f"I{tr}"].number_format = "0%"
plano[f"K{tr}"].number_format = "#,##0.00"
plano.row_dimensions[tr].height = 26

# validações
dv_malha = DataValidation(type="list", formula1='"Algodão Pima Preta,Peruana Off White"',
                          allow_blank=True,
                          prompt="Escolha a malha. Para incluir outra, cadastre na aba MALHAS.",
                          promptTitle="Malha")
dv_mod = DataValidation(type="list", formula1='"Axozee Slim,Axozee Over"', allow_blank=True)
dv_tam = DataValidation(type="list", formula1='"P,M,G,GG"', allow_blank=True)
dv_resp = DataValidation(type="list", formula1='"Jhonatan,Corte - turno 1,Corte - turno 2,Terceiro"',
                         allow_blank=True)
for dv, rng in ((dv_malha, f"B7:B{ULT}"), (dv_mod, f"C7:C{ULT}"),
                (dv_tam, f"D7:D{ULT}"), (dv_resp, f"M7:M{ULT}")):
    plano.add_data_validation(dv)
    dv.add(rng)

dv_qtd = DataValidation(type="whole", operator="greaterThanOrEqual", formula1=0,
                        allow_blank=True, errorTitle="Quantidade inválida",
                        error="Digite um número inteiro igual ou maior que zero.")
plano.add_data_validation(dv_qtd)
dv_qtd.add(f"F7:G{ULT}")

# formatação condicional de status
status_rng = f"J7:J{ULT}"
for txt, cor, bg in (("CORTADO", VERDE, VERDE_BG), ("EM CORTE", AMBAR, AMBAR_BG),
                     ("PENDENTE", VERM, VERM_BG), ("SEM PLANO", CINZA, CINZA_BG),
                     ("EXCEDENTE", "FFFFFF", TERRACOTA)):
    plano.conditional_formatting.add(status_rng,
        CellIsRule(operator="equal", formula=[f'"{txt}"'],
                   fill=PatternFill("solid", fgColor=bg),
                   font=Font(name=F, bold=True, size=9, color=cor)))

plano.conditional_formatting.add(f"I7:I{ULT}",
    DataBarRule(start_type="num", start_value=0, end_type="num", end_value=1,
                color=OLIVA, showValue=True))
# linhas sem plano ficam apagadas
plano.conditional_formatting.add(f"A7:N{ULT}",
    FormulaRule(formula=["$F7=0"], font=Font(name=F, size=9, color="B3AEA0")))

plano.freeze_panes = "A7"
plano.auto_filter.ref = f"A6:N{ULT}"
plano["G6"].comment = Comment("Única coluna que o corte precisa atualizar no dia a dia. "
                              "O status muda sozinho conforme você digita.", "AXOZEE")

# =====================================================================
# ABA PAINEL
# =====================================================================
titulo(painel, "A1", "AXOZEE  ·  PAINEL DE CORTE", "L1",
       "Tudo nesta aba é calculado. Para mudar números, use o PLANO DE CORTE e MALHAS.")

# --- KPIs ---
KPIS = [
    ("A", "PEÇAS PLANEJADAS", "='PLANO DE CORTE'!F23", "#,##0", AREIA),
    ("D", "PEÇAS CORTADAS",   "='PLANO DE CORTE'!G23", "#,##0", AREIA),
    ("G", "FALTA CORTAR",     "='PLANO DE CORTE'!H23", "#,##0", AREIA),
    ("J", "CONCLUSÃO",        "=IFERROR('PLANO DE CORTE'!G23/'PLANO DE CORTE'!F23,0)", "0%", TERRACOTA),
]
for col, rotulo, formula, fmt, bg in KPIS:
    c2 = get_column_letter(ord(col) - 64 + 1)
    painel[f"{col}4"] = rotulo
    painel.merge_cells(f"{col}4:{c2}4")
    style(painel, f"{col}4:{c2}4", bold=True, size=9, color=AREIA, bg=GRAFITE, align="center")
    painel[f"{col}5"] = formula
    painel.merge_cells(f"{col}5:{c2}5")
    cor_txt = "FFFFFF" if bg == TERRACOTA else ASFALTO
    style(painel, f"{col}5:{c2}5", bold=True, size=26, color=cor_txt, bg=bg,
          align="center", fmt=fmt)
painel.row_dimensions[4].height = 20
painel.row_dimensions[5].height = 46

KPIS2 = [
    ("A", "KG PREVISTOS NO PLANO", "=MALHAS!H7", "#,##0.00"),
    ("D", "KG REAIS CONSUMIDOS",   "=MALHAS!J7", "#,##0.00"),
    ("G", "RENDIMENTO REAL (pç/kg)", "=IFERROR(MALHAS!I7/MALHAS!J7,0)", "#,##0.00"),
    ("J", "GOLAS A CORTAR",        "=MODELAGENS!G5+MODELAGENS!G6", "#,##0"),
]
for col, rotulo, formula, fmt in KPIS2:
    c2 = get_column_letter(ord(col) - 64 + 1)
    painel[f"{col}7"] = rotulo
    painel.merge_cells(f"{col}7:{c2}7")
    style(painel, f"{col}7:{c2}7", bold=True, size=9, color=AREIA, bg=OLIVA, align="center")
    painel[f"{col}8"] = formula
    painel.merge_cells(f"{col}8:{c2}8")
    style(painel, f"{col}8:{c2}8", bold=True, size=18, color=ASFALTO, bg=AREIA_CL,
          align="center", fmt=fmt)
painel.row_dimensions[7].height = 20
painel.row_dimensions[8].height = 34

for col, w in zip("ABCDEFGHIJKL", [23, 12, 13, 12, 13, 12, 13, 12, 18, 12, 13, 12]):
    painel.column_dimensions[col].width = w

# --- tabela por malha ---
painel["A11"] = "POR MALHA"
painel.merge_cells("A11:F11")
style(painel, "A11:F11", bold=True, size=11, color=AREIA, bg=ASFALTO, align="left")
cabecalho(painel, 12, 1, ["Malha", "Planejado", "Cortado", "Falta", "% cortado", "Kg previstos"],
          [16, 11, 11, 10, 11, 12])
for i in range(2):
    r = 13 + i
    m = 5 + i
    painel[f"A{r}"] = f"=MALHAS!B{m}&\" \"&MALHAS!C{m}"
    painel[f"B{r}"] = f"=MALHAS!G{m}"
    painel[f"C{r}"] = f"=MALHAS!I{m}"
    painel[f"D{r}"] = f"=MAX(0,B{r}-C{r})"
    painel[f"E{r}"] = f"=IFERROR(C{r}/B{r},0)"
    painel[f"F{r}"] = f"=MALHAS!H{m}"
    painel.row_dimensions[r].height = 22
style(painel, "A13:A14", bold=True, size=10, align="left")
style(painel, "B13:D14", size=10, align="center", fmt="#,##0")
style(painel, "E13:E14", size=10, align="center", fmt="0%")
style(painel, "F13:F14", size=10, align="center", fmt="#,##0.00")
painel.conditional_formatting.add("E13:E14",
    DataBarRule(start_type="num", start_value=0, end_type="num", end_value=1,
                color=TERRACOTA, showValue=True))

# --- tabela por modelagem ---
painel["H11"] = "POR MODELAGEM"
painel.merge_cells("H11:L11")
style(painel, "H11:L11", bold=True, size=11, color=AREIA, bg=ASFALTO, align="left")
cabecalho(painel, 12, 8, ["Modelagem", "Planejado", "Cortado", "% cortado", "Golas"],
          [16, 11, 11, 11, 10])
for i in range(2):
    r = 13 + i
    m = 5 + i
    painel[f"H{r}"] = f"=MODELAGENS!A{m}"
    painel[f"I{r}"] = f"=MODELAGENS!E{m}"
    painel[f"J{r}"] = f"=MODELAGENS!F{m}"
    painel[f"K{r}"] = f"=IFERROR(J{r}/I{r},0)"
    painel[f"L{r}"] = f"=MODELAGENS!G{m}"
style(painel, "H13:H14", bold=True, size=10, align="left")
style(painel, "I13:J14", size=10, align="center", fmt="#,##0")
style(painel, "K13:K14", size=10, align="center", fmt="0%")
style(painel, "L13:L14", size=10, align="center", fmt="#,##0")
painel.conditional_formatting.add("K13:K14",
    DataBarRule(start_type="num", start_value=0, end_type="num", end_value=1,
                color=OLIVA, showValue=True))

# --- tabela por tamanho ---
painel["A17"] = "POR TAMANHO"
painel.merge_cells("A17:F17")
style(painel, "A17:F17", bold=True, size=11, color=AREIA, bg=ASFALTO, align="left")
cabecalho(painel, 18, 1, ["Tamanho", "Planejado", "Cortado", "Falta", "% cortado", "Status"],
          [16, 11, 11, 10, 11, 12])
for i, tam in enumerate(["P", "M", "G", "GG"]):
    r = 19 + i
    painel[f"A{r}"] = tam
    painel[f"B{r}"] = f"=SUMIFS('PLANO DE CORTE'!$F$7:$F$22,'PLANO DE CORTE'!$D$7:$D$22,$A{r})"
    painel[f"C{r}"] = f"=SUMIFS('PLANO DE CORTE'!$G$7:$G$22,'PLANO DE CORTE'!$D$7:$D$22,$A{r})"
    painel[f"D{r}"] = f"=MAX(0,B{r}-C{r})"
    painel[f"E{r}"] = f"=IFERROR(C{r}/B{r},0)"
    painel[f"F{r}"] = f'=IF(B{r}=0,"SEM PLANO",IF(C{r}=0,"PENDENTE",IF(C{r}<B{r},"EM CORTE","CORTADO")))'
    painel.row_dimensions[r].height = 22
style(painel, "A19:A22", bold=True, size=11, align="center")
style(painel, "B19:D22", size=10, align="center", fmt="#,##0")
style(painel, "E19:E22", size=10, align="center", fmt="0%")
style(painel, "F19:F22", bold=True, size=9, align="center")
for txt, cor, bg in (("CORTADO", VERDE, VERDE_BG), ("EM CORTE", AMBAR, AMBAR_BG),
                     ("PENDENTE", VERM, VERM_BG), ("SEM PLANO", CINZA, CINZA_BG)):
    painel.conditional_formatting.add("F19:F22",
        CellIsRule(operator="equal", formula=[f'"{txt}"'],
                   fill=PatternFill("solid", fgColor=bg),
                   font=Font(name=F, bold=True, size=9, color=cor)))

# --- rendimento ---
painel["H17"] = "RENDIMENTO DA MALHA"
painel.merge_cells("H17:L17")
style(painel, "H17:L17", bold=True, size=11, color=AREIA, bg=ASFALTO, align="left")
cabecalho(painel, 18, 8, ["Malha", "Previsto", "Real", "Desvio", "Saldo kg"],
          [16, 11, 11, 11, 10])
for i in range(2):
    r = 19 + i
    m = 5 + i
    painel[f"H{r}"] = f"=MALHAS!B{m}"
    painel[f"I{r}"] = f"=MALHAS!E{m}"
    painel[f"J{r}"] = f"=MALHAS!K{m}"
    painel[f"K{r}"] = f'=IF(MALHAS!J{m}=0,"—",MALHAS!L{m})'
    painel[f"L{r}"] = f"=MALHAS!M{m}"
style(painel, "H19:H20", bold=True, size=10, align="left")
style(painel, "I19:J20", size=10, align="center", fmt="#,##0.00")
style(painel, "K19:K20", bold=True, size=10, align="center", fmt="+0.0%;-0.0%;0.0%")
style(painel, "L19:L20", size=10, align="center", fmt="#,##0.00")
painel.conditional_formatting.add("K19:K20",
    CellIsRule(operator="lessThan", formula=["0"],
               font=Font(name=F, bold=True, color=VERM), fill=PatternFill("solid", fgColor=VERM_BG)))
painel.conditional_formatting.add("K19:K20",
    CellIsRule(operator="greaterThan", formula=["0"],
               font=Font(name=F, bold=True, color=VERDE), fill=PatternFill("solid", fgColor=VERDE_BG)))

painel["H21"] = ("Rendimento real só aparece depois que você lançar os quilos realmente "
                 "consumidos na aba MALHAS.")
painel.merge_cells("H21:L22")
style(painel, "H21:L22", size=9, italic=True, color=CINZA, bg=AREIA_CL, align="left",
      wrap=True, border=False)

# --- gráficos ---
g1 = BarChart()
g1.type = "col"
g1.style = 2
g1.title = "Planejado x Cortado por tamanho"
g1.y_axis.title = "Peças"
g1.x_axis.title = "Tamanho"
dados = Reference(painel, min_col=2, max_col=3, min_row=18, max_row=22)
cats = Reference(painel, min_col=1, min_row=19, max_row=22)
g1.add_data(dados, titles_from_data=True)
g1.set_categories(cats)
g1.height, g1.width = 8.5, 16
g1.series[0].graphicalProperties.solidFill = OLIVA
g1.series[1].graphicalProperties.solidFill = TERRACOTA
painel.add_chart(g1, "A25")

g2 = BarChart()
g2.type = "bar"
g2.style = 2
g2.title = "Planejado x Cortado por malha"
dados2 = Reference(painel, min_col=2, max_col=3, min_row=12, max_row=14)
cats2 = Reference(painel, min_col=1, min_row=13, max_row=14)
g2.add_data(dados2, titles_from_data=True)
g2.set_categories(cats2)
g2.height, g2.width = 8.5, 16
g2.series[0].graphicalProperties.solidFill = OLIVA
g2.series[1].graphicalProperties.solidFill = TERRACOTA
painel.add_chart(g2, "H25")

# dados auxiliares do gráfico de rosca (contagem de status)
painel["N44"] = "Status"
painel["O44"] = "Grades"
for i, st in enumerate(["CORTADO", "EM CORTE", "PENDENTE", "EXCEDENTE"]):
    painel[f"N{45+i}"] = st
    painel[f"O{45+i}"] = f'=COUNTIF(\'PLANO DE CORTE\'!$J$7:$J$22,"{st}")'
style(painel, "N44:O48", size=9, align="center", color=CINZA)
painel.column_dimensions["N"].width = 14
painel.column_dimensions["O"].width = 9
painel["N43"] = "tabela auxiliar do gráfico — não apagar"
style(painel, "N43:O43", size=8, italic=True, color=CINZA, border=False)

g3 = DoughnutChart()
g3.title = "Grades por status"
d3 = Reference(painel, min_col=15, min_row=44, max_row=48)
c3 = Reference(painel, min_col=14, min_row=45, max_row=48)
g3.add_data(d3, titles_from_data=True)
g3.set_categories(c3)
g3.height, g3.width = 8.5, 11
painel.add_chart(g3, "A44")

painel.sheet_view.showGridLines = False

# =====================================================================
# ABA COMO USAR
# =====================================================================
titulo(ajuda, "A1", "COMO USAR ESTA PLANILHA", "F1",
       "Leia uma vez. Depois é só preencher o amarelo.")
ajuda.column_dimensions["A"].width = 4
for col, w in zip("BCDEF", [26, 26, 26, 26, 26]):
    ajuda.column_dimensions[col].width = w

LINHAS = [
    ("TÍTULO", "O fluxo do dia", None),
    ("PASSO", "1. Mande esta planilha para o corte",
     "O corte trabalha na aba PLANO DE CORTE. Cada linha é uma grade: malha + modelagem + tamanho."),
    ("PASSO", "2. O corte preenche só três colunas",
     "Qtd cortada, Data do corte e Responsável. O status muda sozinho conforme a quantidade."),
    ("PASSO", "3. Depois do corte, lance os quilos reais",
     "Na aba MALHAS, coluna 'Kg real consumido'. É o que revela o rendimento real contra os 5 e 4,5 pç/kg previstos."),
    ("PASSO", "4. Acompanhe no PAINEL",
     "Tudo lá é automático: peças, percentuais, rendimento, gráficos."),
    ("TÍTULO", "Cores das células", None),
    ("COR_INPUT", "Amarelo = você preenche",
     "São as únicas células que precisam de digitação."),
    ("COR_CALC", "Branco = fórmula",
     "Calculado sozinho. Se sobrescrever, a automação daquela linha se perde."),
    ("COR_HEAD", "Verde-oliva = cabeçalho",
     "Nome das colunas."),
    ("TÍTULO", "Os status", None),
    ("ST_PEND", "PENDENTE", "Tem plano, nada cortado ainda."),
    ("ST_CORT", "EM CORTE", "Cortou parte da grade."),
    ("ST_OK", "CORTADO", "Quantidade cortada bateu com a planejada."),
    ("ST_EXC", "EXCEDENTE", "Cortou mais do que o planejado — conferir sobra de malha."),
    ("ST_SEM", "SEM PLANO", "Grade existe na tabela mas está zerada nesta rodada."),
    ("TÍTULO", "Regras de gola (automáticas)", None),
    ("REGRA", "Axozee Slim → corta gola",
     "Gola da própria malha, risco já incluso na modelagem."),
    ("REGRA", "Axozee Over → não corta gola",
     "Ribana canelada aplicada na costura."),
    ("TÍTULO", "Para a próxima rodada", None),
    ("PASSO", "Reaproveitando a planilha",
     "Zere a coluna 'Qtd cortada', ajuste as quantidades planejadas e os quilos na aba MALHAS. "
     "A grade de 16 linhas já cobre as duas malhas × duas modelagens × quatro tamanhos."),
    ("PASSO", "Incluindo uma malha nova",
     "Acrescente a linha na aba MALHAS com um código novo (M3) e use esse código na coluna "
     "'Malha' do plano. Amplie as fórmulas de busca até a nova linha."),
]

r = 4
for tipo, txt, desc in LINHAS:
    if tipo == "TÍTULO":
        ajuda[f"B{r}"] = txt
        ajuda.merge_cells(f"B{r}:F{r}")
        style(ajuda, f"B{r}:F{r}", bold=True, size=12, color=AREIA, bg=ASFALTO, align="left")
        ajuda.row_dimensions[r].height = 26
        r += 1
        continue
    ajuda[f"B{r}"] = txt
    ajuda[f"C{r}"] = desc or ""
    ajuda.merge_cells(f"C{r}:F{r}")
    mapa = {
        "COR_INPUT": (INPUT_BG, ASFALTO), "COR_CALC": ("FFFFFF", ASFALTO),
        "COR_HEAD": (OLIVA, AREIA), "ST_PEND": (VERM_BG, VERM), "ST_CORT": (AMBAR_BG, AMBAR),
        "ST_OK": (VERDE_BG, VERDE), "ST_EXC": (TERRACOTA, "FFFFFF"), "ST_SEM": (CINZA_BG, CINZA),
        "REGRA": (AREIA, ASFALTO), "PASSO": (AREIA_CL, ASFALTO),
    }
    bg, fg = mapa[tipo]
    style(ajuda, f"B{r}:B{r}", bold=True, size=10, color=fg, bg=bg, align="left", wrap=True)
    style(ajuda, f"C{r}:F{r}", size=10, color="333333", align="left", wrap=True)
    ajuda.row_dimensions[r].height = 30
    r += 1

ajuda[f"B{r+1}"] = ("Dados desta versão vieram de você em 13/10/2026: Algodão Pima Preta 12 kg a "
                    "5 pç/kg; Peruana Off White 18 kg a 4,5 pç/kg; grades Slim e Over conforme "
                    "informado. Capacidade da Peruana é 81 peças e o plano usa 80 — sobra 1 peça "
                    "(0,22 kg).")
ajuda.merge_cells(f"B{r+1}:F{r+2}")
style(ajuda, f"B{r+1}:F{r+2}", size=9, italic=True, color=CINZA, bg=AREIA_CL,
      align="left", wrap=True, border=False)
ajuda.sheet_view.showGridLines = False

for ws in (painel, plano, malhas, model, ajuda):
    ws.sheet_properties.tabColor = TERRACOTA

# configuração de impressão (o plano é o que vai impresso para o corte)
for ws, area in ((plano, f"A1:N{ULT+1}"), (malhas, f"A1:N{ULT_MAL+3}"), (model, f"A1:G{ULT_MOD+2}"), (painel, "A1:L52")):
    ws.page_setup.orientation = "landscape"
    ws.page_setup.fitToWidth = 1
    ws.page_setup.fitToHeight = 0
    ws.sheet_properties.pageSetUpPr.fitToPage = True
    ws.print_area = area
    ws.print_options.horizontalCentered = True
    ws.page_margins.left = ws.page_margins.right = 0.3
plano.print_title_rows = "6:6"
plano.oddFooter.right.text = "Página &P de &N"
plano.oddFooter.left.text = "AXOZEE · Plano de Corte · Cápsula 01"
plano.oddFooter.center.text = "Impresso em &D"

wb.save(OUT)
print("salvo:", OUT)
