#! /usr/bin/env python3

###
# Author: Anthony Ishkan (anthony.ishkan@gmail.com, https://github.com/ishkanan)
#
# Generates passwords for "Castlevania II - Simon's Quest" on the NES.
# Feel free to improve and consider submitting a PR to https://github.com/bisqwit/password_codecs
#
# Big shout-out to Bisqwit (https://www.youtube.com/channel/UCKTehwyGCKF-b2wo0RKwrcg)
# for his detailed breakdown of the logic and the base conversion code.
# Check his awesome video out at https://www.youtube.com/watch?v=_3ve0YEQEMw&t=205s
###

from functools import reduce


NTSC_LETTERS = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'

NTSC_TO_PAL_LETTER_MAP = {
    'A': 'B', 'B': 'D', 'C': 'G', 'D': 'H', 'E': 'J', 'F': 'K',
    'G': 'L', 'H': 'M', 'I': 'N', 'J': 'P', 'K': 'Q', 'L': 'R',
    'M': 'T', 'N': 'V', 'O': 'W', 'P': 'X', 'Q': 'Y', 'R': 'Z',
    'S': '0', 'T': '1', 'U': '2', 'V': '3', 'W': '4', 'X': '5',
    'Y': '6', 'Z': '7', '0': '8', '1': '9', '2': '♦', '3': '♠',
    '4': '♥', '5': '@', '6': '★', '7': '#', '8': '!', '9': '?',
}

XOR_TABLE = [14, 1, 10, 16, 4, 15, 24, 27, 22, 7, 30, 18, 17, 29]


def convert_base(elements, from_bitsize, to_bitsize):
    cache = 0
    cachelen = 0
    result = []
    obm = (1 << to_bitsize) - 1
    for elem in elements:
        cache = (cache << from_bitsize) + elem
        cachelen += from_bitsize
        while (cachelen >= to_bitsize):
            cachelen -= to_bitsize
            result.append((cache >> cachelen) & obm)
            cache &= (1 << cachelen) - 1
    return result


def day_to_bcd(day):
    bcd = []
    day_string = '0' + str(day) if day < 10 else str(day)
    for digit in day_string:
        bcd.append(int(digit))
    return bcd


def bools_to_byte(*bools):
    # highest order bit is first
    bools = list(bools)
    bools.reverse()
    result = 0
    for i in range(len(bools)):
        result |= int(bools[i]) << i
    return result


def get_player_state():
    def clamp(n, minn, maxn):
        return max(min(maxn, n), minn)
    level = clamp(int(input('Player level (0-6): ')), 0, 6)
    day = clamp(int(input('Day number (0-99): ')), 0, 99)
    silk_bag = input('Silk bag (y/n): ').lower() == 'y'
    cross = input('Magic cross (y/n): ').lower() == 'y'
    max_laurels = 8 if silk_bag else 4
    laurels = clamp(int(input(f'Laurels (0-{max_laurels}): ')), 0, max_laurels)
    garlics = clamp(int(input('Garlics (0-8): ')), 0, 8)
    oak_stake = input('Oak stake (y/n): ').lower() == 'y'
    sacred_flame = input('Sacred flame (y/n): ').lower() == 'y'
    diamond = input('Diamond (y/n): ').lower() == 'y'
    holy_water = input('Holy water (y/n): ').lower() == 'y'
    silver_knife = input('Silver knife (y/n): ').lower() == 'y'
    gold_knife = input('Gold knife (y/n): ').lower() == 'y'
    dagger = input('Dagger (y/n): ').lower() == 'y'
    crystal = input('Crystal (none/w/b/r): ').lower()
    crystal = {'w': 1, 'b': 2, 'r': 3}.get(crystal, 0)
    whip = input('Whip (l/t/c/m/f): ').lower()
    whip = {'t': 1, 'c': 2, 'm': 3, 'f': 4}.get(whip, 0)
    rib = input("Dracula's rib (y/n): ").lower() == 'y'
    heart = input("Dracula's heart (y/n): ").lower() == 'y'
    eye = input("Dracula's eye (y/n): ").lower() == 'y'
    nail = input("Dracula's nail (y/n): ").lower() == 'y'
    ring = input("Dracula's ring (y/n): ").lower() == 'y'

    return {
        'level': level,
        'day': day,
        'silk_bag': silk_bag,
        'cross': cross,
        'laurels': laurels,
        'garlics': garlics,
        'oak_stake': oak_stake,
        'sacred_flame': sacred_flame,
        'diamond': diamond,
        'holy_water': holy_water,
        'silver_knife': silver_knife,
        'gold_knife': gold_knife,
        'dagger': dagger,
        'crystal': crystal,
        'whip': whip,
        'rib': rib,
        'heart': heart,
        'eye': eye,
        'nail': nail,
        'ring': ring,
    }


if __name__ == '__main__':
    print('"Castlevania II - Simon\'s Quest" Password Generator\n')

    state = get_player_state()

    day_bcd = day_to_bcd(state['day'])
    data_bytes = [
        state['level'],
        (int(day_bcd[0]) << 4) | int(day_bcd[1]),
        (state['garlics'] << 4) | state['laurels'],
        (state['crystal'] << 5) | bools_to_byte(state['ring'], state['nail'], state['eye'], state['heart'], state['rib']),
        bools_to_byte(state['garlics'] > 0, state['laurels'] > 0, state['cross'], state['silk_bag']),
        bools_to_byte(state['oak_stake'], state['sacred_flame'], state['diamond'], state['holy_water'], state['gold_knife'], state['silver_knife'], state['dagger']),
        state['whip'],
    ]
    checksum = reduce(lambda sum, item: sum + item, data_bytes)
    data_bytes += [
        checksum >> 16,
        checksum & 0xFFFF,
        0,
    ]

    data_letters = convert_base(data_bytes, 8, 5)
    for i in range(14):
        data_letters[i] ^= XOR_TABLE[i]
    ntsc_letters = reduce(lambda pw, letter: pw + NTSC_LETTERS[letter], data_letters, '')
    pal_letters = reduce(lambda pw, letter: pw + NTSC_TO_PAL_LETTER_MAP[letter], ntsc_letters, '')

    def pretty_pw(pw):
        return f'{pw[0:4]} {pw[4:8]} {pw[8:12]} {pw[12:]}'

    print(f'\nNTSC password is: {pretty_pw(ntsc_letters)}')
    print(f'PAL password is: {pretty_pw(pal_letters)}')
