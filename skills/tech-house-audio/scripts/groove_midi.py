#!/usr/bin/env python3
"""Transforma um MIDI de baixo: gate, oitava, acento e micro-timing.

    python3 tools/groove_midi.py entrada.mid saida.mid --bpm 126 --gate 1.15
    python3 tools/groove_midi.py entrada.mid saida.mid --oitava -1 --gate 1.2
    python3 tools/groove_midi.py --demo demo.mid --bpm 126     # gera exemplo

Cada transformação é determinística e o efeito de cada uma está escrito. Rode
`--relatorio` para ver o antes e depois em números.

O QUE ESTA FERRAMENTA NÃO FAZ: julgar se o resultado tem groove. Ela move notas
segundo regras declaradas; se soa melhor é você quem decide.
"""

import argparse
import sys

import mido


# --------------------------------------------------------------------------
# Leitura
# --------------------------------------------------------------------------

def ler_notas(mid):
    """Extrai notas como (inicio_tick, duracao_tick, nota, velocity, canal).

    Casa cada note_on com o note_off correspondente, tratando note_on de
    velocity 0 como note_off -- que é como muitos sequenciadores escrevem.
    """
    notas, abertas, t = [], {}, 0
    for msg in mido.merge_tracks(mid.tracks):
        t += msg.time
        if msg.type == "note_on" and msg.velocity > 0:
            abertas.setdefault((msg.channel, msg.note), []).append((t, msg.velocity))
        elif msg.type == "note_off" or (msg.type == "note_on" and msg.velocity == 0):
            k = (msg.channel, msg.note)
            if abertas.get(k):
                ini, vel = abertas[k].pop(0)
                notas.append([ini, max(1, t - ini), msg.note, vel, msg.channel])
    notas.sort(key=lambda n: (n[0], n[2]))
    return notas


def escrever(notas, tpb, bpm, caminho):
    ev = []
    for ini, dur, nota, vel, ch in notas:
        ev.append((ini, 1, mido.Message("note_on", note=nota, velocity=vel, channel=ch, time=0)))
        ev.append((ini + dur, 0, mido.Message("note_off", note=nota, velocity=0, channel=ch, time=0)))
    ev.sort(key=lambda e: (e[0], e[1]))   # note_off antes de note_on no mesmo tick

    mid = mido.MidiFile(type=0, ticks_per_beat=tpb)
    tr = mido.MidiTrack()
    mid.tracks.append(tr)
    tr.append(mido.MetaMessage("set_tempo", tempo=mido.bpm2tempo(bpm), time=0))
    ant = 0
    for tick, _, msg in ev:
        msg.time = max(0, tick - ant)
        ant = tick
        tr.append(msg)
    mid.save(caminho)


# --------------------------------------------------------------------------
# Transformações
# --------------------------------------------------------------------------

def aplicar_gate(notas, tpb, alvo, minimo_ms=None, bpm=126.0, legato=False):
    """Define a duração de cada nota como uma fração da semicolcheia.

    Numa linha de sub isto é o parâmetro mais importante e o menos óbvio: o
    ouvido precisa de cerca de QUATRO CICLOS da fundamental para perceber
    altura. A 45 Hz um ciclo dura 22 ms, então uma nota de 80 ms mal chega a
    3,6 ciclos -- ela vira um pulso sem nota. Descer a linha uma oitava sem
    alongar as notas troca grave por batida surda.
    """
    semi = tpb / 4.0
    minimo_ticks = 0
    if minimo_ms:
        minimo_ticks = int(minimo_ms / 1000.0 * (bpm / 60.0) * tpb)

    for i, n in enumerate(notas):
        alvo_ticks = int(semi * alvo)
        if legato and i + 1 < len(notas):
            # nunca invade a próxima nota por mais do que uma sobreposição curta
            espaco = notas[i + 1][0] - n[0]
            alvo_ticks = min(alvo_ticks, int(espaco * 1.05))
        n[1] = max(alvo_ticks, minimo_ticks, 1)
    return notas


def acentuar(notas, tpb, forte=112, fraco=88, posicoes_fortes=(2, 6, 10, 14)):
    """Acento por posição no compasso, não aleatório.

    Velocity sorteada soa nervosa, não humana: um músico acentua sempre nos
    mesmos lugares da frase. O padrão medido no baixo do produtor já usa as
    semicolcheias fora do tempo forte, então o acento vai nelas.
    """
    semi = tpb / 4.0
    for n in notas:
        passo = int(round(n[0] / semi)) % 16
        n[3] = forte if passo in posicoes_fortes else fraco
    return notas


def micro_timing(notas, tpb, atraso_ms, bpm, apenas_contratempo=True):
    """Atrasa notas em milissegundos.

    NÃO calibrado contra referência: as três faixas de referência foram medidas
    e o resultado ficou dentro da incerteza do detector (~25 ms para conteúdo
    grave, porque um ciclo de 50 Hz já dura 20 ms). Portanto este parâmetro é
    escolha de gosto, não alvo medido. Comece em 0 e suba de 3 em 3 ms.
    """
    if abs(atraso_ms) < 0.01:
        return notas
    semi = tpb / 4.0
    ticks = atraso_ms / 1000.0 * (bpm / 60.0) * tpb
    for n in notas:
        passo = int(round(n[0] / semi))
        if (not apenas_contratempo) or (passo % 2 == 1):
            n[0] = max(0, int(n[0] + ticks))
    return notas


def transpor(notas, semitons):
    for n in notas:
        n[2] = max(0, min(127, n[2] + semitons))
    return notas


def quebrar_repeticao(notas, tpb, reducao=0.55):
    """Encurta a segunda de duas notas iguais e seguidas.

    Nota idêntica repetida com mesmo gate e mesma velocity é a assinatura mais
    audível de MIDI programado. Variar a segunda basta para tirar isso.
    """
    for i in range(1, len(notas)):
        a, b = notas[i - 1], notas[i]
        if a[2] == b[2] and (b[0] - a[0]) <= tpb / 2:
            b[1] = max(1, int(b[1] * reducao))
            b[3] = max(1, min(127, b[3] - 12))
    return notas


# --------------------------------------------------------------------------

def relatorio(notas, tpb, bpm, titulo):
    if not notas:
        print(f"{titulo}: sem notas"); return
    semi = tpb / 4.0
    gates = [n[1] / semi for n in notas]
    vels = [n[3] for n in notas]
    ms_por_semi = 60000.0 / bpm / 4
    gate_med = sorted(gates)[len(gates) // 2]
    f0 = [440 * 2 ** ((n[2] - 69) / 12) for n in notas]
    f0_med = sorted(f0)[len(f0) // 2]
    dur_ms = gate_med * ms_por_semi
    print(f"\n  {titulo}")
    print(f"    notas                {len(notas)}")
    print(f"    gate mediano         {gate_med:.2f} semicolcheia  ({dur_ms:.0f} ms)")
    print(f"    velocity             {min(vels)} a {max(vels)}")
    print(f"    fundamental mediana  {f0_med:.1f} Hz")
    print(f"    ciclos por nota      {dur_ms / (1000.0 / f0_med):.1f}"
          f"   {'<<< menos de 4: o ouvido não forma altura' if dur_ms / (1000.0 / f0_med) < 4 else 'ok'}")


def demo(tpb):
    """Padrão medido no stem de baixo do produtor: .XX..XXx.xXX.XXx

    Notas em Fá#/Sol na oitava original, para o antes-e-depois ser comparável
    com o que ele já tem na sessão.
    """
    passos = [1, 2, 5, 6, 7, 9, 10, 11, 13, 14, 15]
    alturas = [42, 42, 43, 42, 40, 42, 43, 45, 42, 43, 42]
    semi = tpb // 4
    notas = []
    for compasso in range(4):
        for p, alt in zip(passos, alturas):
            notas.append([compasso * tpb * 4 + p * semi, int(semi * 0.68), alt, 100, 0])
    return notas


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("entrada", nargs="?")
    ap.add_argument("saida", nargs="?", default="groove.mid")
    ap.add_argument("--demo", metavar="ARQ", help="gera um exemplo em vez de ler entrada")
    ap.add_argument("--bpm", type=float, default=126.0)
    ap.add_argument("--gate", type=float, default=1.15,
                    help="duração alvo em semicolcheias (padrão 1.15)")
    ap.add_argument("--min-ms", type=float, default=90.0,
                    help="duração mínima em ms, para o sub formar altura")
    ap.add_argument("--oitava", type=int, default=0, help="-1 desce uma oitava")
    ap.add_argument("--atraso-ms", type=float, default=0.0,
                    help="atraso do contratempo (não calibrado, use por gosto)")
    ap.add_argument("--forte", type=int, default=112)
    ap.add_argument("--fraco", type=int, default=88)
    ap.add_argument("--legato", action="store_true", help="deixa notas encostarem")
    ap.add_argument("--sem-acento", action="store_true")
    a = ap.parse_args()

    tpb = 480
    if a.demo:
        notas = demo(tpb)
        saida = a.demo
    else:
        if not a.entrada:
            ap.error("passe um arquivo .mid de entrada, ou use --demo")
        mid = mido.MidiFile(a.entrada)
        tpb = mid.ticks_per_beat
        notas = ler_notas(mid)
        saida = a.saida
        if not notas:
            print("nenhuma nota encontrada no arquivo"); return 1

    relatorio(notas, tpb, a.bpm, "ANTES")

    if a.oitava:
        notas = transpor(notas, 12 * a.oitava)
    notas = aplicar_gate(notas, tpb, a.gate, a.min_ms, a.bpm, a.legato)
    if not a.sem_acento:
        notas = acentuar(notas, tpb, a.forte, a.fraco)
    notas = micro_timing(notas, tpb, a.atraso_ms, a.bpm)
    notas = quebrar_repeticao(notas, tpb)

    relatorio(notas, tpb, a.bpm, "DEPOIS")
    escrever(notas, tpb, a.bpm, saida)
    print(f"\n  gravado: {saida}")
    print(f"  arraste para uma pista MIDI do Ableton.\n")
    print(f"  Nada aqui julga se ficou com groove — as regras são declaradas e")
    print(f"  determinísticas. Quem ouve é você.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
