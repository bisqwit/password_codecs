#include <string_view>
#include <functional>
#include <algorithm>
#include <numeric>
#include <limits>
#include <array>
#include <tuple>

namespace /* Some utility functions */
{
    /* Rotate a range of integers left by given number of bits. */
    // Tip: To rotate right by n bits, use (range_length * unit - n_bits) as the shift count.
    //      If range_length*unit is a power of two, then passing -n_bits will work too.
    template<typename I, typename It = std::iterator_traits<I>,
             // unit = number of bits in the pointer integer type
             unsigned unit = std::numeric_limits<typename It::value_type>::digits,
             // enable this overload only if I is a bidirectional or random iterator
             typename = std::enable_if_t<std::is_base_of_v<std::bidirectional_iterator_tag, typename It::iterator_category>>,
             // and "unit" must be measured in bits…
             typename = std::enable_if_t<std::numeric_limits<typename It::value_type>::radix == 2>>
    void rotl(I begin, I end, unsigned n_bits)
    {
        // First rotate by full array elements, if n_bits >= unit.
        std::rotate(begin, std::next(begin, n_bits / unit % (unsigned)std::distance(begin,end)), end);

        // Then shift the bits properly. This involves handling all adjacent pairs of units.
        if(unsigned bi = n_bits % unit; bi && begin != end)
            for(auto prev = *begin; begin != end; std::swap(prev, *end))
                prev = (*--end << bi) | (prev >> (unit - bi));
    }

    /* Create a 64-bit bitmask where N lsb bits are all “1”; 0 ≤ N ≤ 63 */
    constexpr std::uint64_t ones(unsigned n)
    {
        return ~(~std::uint64_t{} << n);
    }

    /* Swap bytes in 64-bit word (toggle endianess). Reputable compilers all optimize this into a single BSWAP instruction. */
    std::uint64_t byteswap(std::uint64_t w)
    {
        auto shu = [](auto n,auto w){ auto a = ones(n)*(ones(63)/ones(2*n)*2+1); return ((w&a)<<n)|((w&~a)>>n); };
        return shu(8u, shu(16u, shu(32u, w)));
        // Tip: To turn this into a function that reverses *bits* and not just bytes,
        // just wrap the return value in shu(1u, shu(2u, shu(4u, ‗‗‗))). It’s that simple!
        // It is also trivial to extend this function to operate on e.g. 32-bit or 128-bit integers (if available).
    }


    /* Helper for getting the element type from arrays, including std::array; for non-arrays the type itself is returned. */
    template<typename T> struct elem_type      : public std::remove_cv<T> {};
    template<typename T> struct elem_type<T&>  : public elem_type<T> {};
    template<typename T> struct elem_type<T&&> : public elem_type<T> {};
    template<typename K, std::size_t V> struct elem_type<K[V]>                  : public elem_type<K> {};
    template<typename K, std::size_t V> struct elem_type<std::array<K,V>>       : public elem_type<K> {};
    template<typename K, std::size_t V> struct elem_type<const std::array<K,V>> : public elem_type<K> {};

    /* Calculate checksum of a given memory region. */
    // Works on both big endian and little endian, as long as sizeof(Byte)*Length
    // is a multiple of sizeof(T[0]) or sizeof(T), whichever is applicable,
    // and Byte is either a byte, or the same as T[0] or T, whichever is applicable.
    template<std::size_t Length, typename Byte = std::uint8_t, typename T>
    Byte checksum(const T& data)
    {
        using E = typename elem_type<T>::type;
        static_assert((sizeof(Byte)*Length) % sizeof(E) == 0,      "call is not endianess-safe");
        static_assert(sizeof(Byte) == 1 || std::is_same_v<Byte,E>, "call is not endianess-safe");
        return std::accumulate((const Byte*)&data, (const Byte*)&data + Length, (Byte)0, std::plus{});
    }
}

/* Some specific data for Metroid password system */
static constexpr std::string_view   CharSet("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz?- ", 65);
static constexpr std::uint_fast64_t MagicWords[2] = {0x5CA6D929C72061B3,0x40000000};

/* User-literal operators. These will help clear out make some declarations later */
constexpr auto operator""_bit(unsigned long long value) { return value; }
constexpr auto operator""_bits(unsigned long long value) { return value; }
constexpr auto operator""_value(unsigned long long value) { return value; }
constexpr auto operator""_position(unsigned long long value) { return value; }
constexpr auto operator""_name(const char* p, std::size_t l) { return std::string_view(p,l); }

/* The meat of the encoder/decoder */
struct MetroPass
{
    std::array<std::tuple<std::string_view/*name*/,unsigned/*pos*/,unsigned/*bits*/, std::uint_fast64_t/*data*/>,82> data
    {{
        // Checksum.
        { "sum"_name,             136_position,  8_bits, {} },

        // Encryption key. Values 0-127 produce the exact same outcome as values 128-255,
        // except for bit #7 of this value, and for flipping bit #7 in checksum as well.
        // Note that the game never generates encryption keys that are multiples of 16.
        { "seed"_name,            128_position,  8_bits, {} },

        // Equipment.
        { "has_bombs"_name,        72_position,   1_bit, {} }, // RAM address $6878 bit #0
        { "has_high_jump"_name,    73_position,   1_bit, {} }, // RAM address $6878 bit #1
        { "has_long_beam"_name,    74_position,   1_bit, {} }, // RAM address $6878 bit #2
        { "has_screw_attack"_name, 75_position,   1_bit, {} }, // RAM address $6878 bit #3
        { "has_morph_ball"_name,   76_position,   1_bit, {} }, // RAM address $6878 bit #4
        { "has_varia_suit"_name,   77_position,   1_bit, {} }, // RAM address $6878 bit #5
        { "has_wave_beam"_name,    78_position,   1_bit, {} }, // RAM address $6878 bit #6
        { "has_ice_beam"_name,     79_position,   1_bit, {} }, // RAM address $6878 bit #7
        // Leotard = what she wears when you unlock the suitless mode.
        { "has_leotard"_name,      71_position,   1_bit, {} }, // RAM address $69B3

        // Collected items. Note that marking any of these items “collected”
        // without setting the equivalent equipment flag will effectively
        // remove the entire item from the game.
        { "taken_bol_104E"_name,    0_position,   1_bit, {} }, // Morph Ball   @  2,14
        { "taken_bom_0325"_name,    6_position,   1_bit, {} }, // Bombs        @ 25, 5
        { "taken_jum_0771"_name,   24_position,   1_bit, {} }, // High Jump    @ 27,17
        { "taken_scr_0DF0"_name,   26_position,   1_bit, {} }, // Screw Attack @ 15,16
        { "taken_var_15E2"_name,   11_position,   1_bit, {} }, // Varia Suit   @ 15, 2

        // Missile door unlocking bits:
        { "door_open_28E5"_name,    2_position,   1_bit, {} }, // Red door @  7, 5 (Red door to long beam)
        { "door_open_2882"_name,    3_position,   1_bit, {} }, // Red door @  4, 2 (Red door to Tourian elevator)
        { "door_open_2B25"_name,    5_position,   1_bit, {} }, // Red door @ 25, 5 (Red door to bombs)
        { "door_open_2A69"_name,    7_position,   1_bit, {} }, // Red door @ 19, 9 (Red door to ice beam)
        { "door_open_29E2"_name,   10_position,   1_bit, {} }, // Red door @ 15, 2 (Red door to varia suit)
        { "door_open_2B4C"_name,   15_position,   1_bit, {} }, // Red door @ 26,12 (Red door to ice beam)
        { "door_open_2B71"_name,   23_position,   1_bit, {} }, // Red door @ 27,17 (Red door to high jump)
        { "door_open_29F0"_name,   25_position,   1_bit, {} }, // Red door @ 15,16 (Red door to screw attack)
        { "door_open_2A55"_name,   29_position,   1_bit, {} }, // Red door @ 18,21 (Red door to wave beam)
        { "door_open_28F5"_name,   32_position,   1_bit, {} }, // Red door @  7,21
        { "door_open_28F6"_name,   35_position,   1_bit, {} }, // Red door @  7,22
        { "door_open_28F8"_name,   37_position,   1_bit, {} }, // Red door @  7,24
        { "door_open_287B"_name,   38_position,   1_bit, {} }, // Red door @  3,27
        { "door_open_291D"_name,   41_position,   1_bit, {} }, // Red door @  8,29 (Red door to Kraid)
        { "door_open_2A39"_name,   44_position,   1_bit, {} }, // Red door @ 17,25
        { "door_open_2A1D"_name,   47_position,   1_bit, {} }, // Red door @ 16,29 (Purple door — Ridley’s room)
        { "door_open_2867"_name,   50_position,   1_bit, {} }, // Red door @  3, 7 (Tourian Door 1: Orange door)
        { "door_open_2927"_name,   51_position,   1_bit, {} }, // Red door @  9, 7 ◆ (Tourian Door 2: Red door)
        { "door_open_292B"_name,   52_position,   1_bit, {} }, // Red door @  9,11 ◆ (Tourian Door 3: Red door)

        // Missile pickups. Note that each of these bits adds +5 to the maximum missile count at $687A.
        { "taken_msl_264B"_name,    1_position,   1_bit, {} }, // Missiles @ 18,11
        { "taken_msl_2703"_name,    8_position,   1_bit, {} }, // Missiles @ 24, 3
        { "taken_msl_264E"_name,   13_position,   1_bit, {} }, // Missiles @ 18,14
        { "taken_msl_262F"_name,   14_position,   1_bit, {} }, // Missiles @ 17,15
        { "taken_msl_276A"_name,   16_position,   1_bit, {} }, // Missiles @ 27,10
        { "taken_msl_278A"_name,   17_position,   1_bit, {} }, // Missiles @ 28,10
        { "taken_msl_278B"_name,   18_position,   1_bit, {} }, // Missiles @ 28,11
        { "taken_msl_276B"_name,   19_position,   1_bit, {} }, // Missiles @ 27,11
        { "taken_msl_274B"_name,   20_position,   1_bit, {} }, // Missiles @ 26,11
        { "taken_msl_268F"_name,   21_position,   1_bit, {} }, // Missiles @ 20,15
        { "taken_msl_266F"_name,   22_position,   1_bit, {} }, // Missiles @ 19,15
        { "taken_msl_2676"_name,   27_position,   1_bit, {} }, // Missiles @ 19,22
        { "taken_msl_2696"_name,   28_position,   1_bit, {} }, // Missiles @ 20,22
        { "taken_msl_2794"_name,   31_position,   1_bit, {} }, // Missiles @ 28,20
        { "taken_msl_2535"_name,   33_position,   1_bit, {} }, // Missiles @  9,21
        { "taken_msl_2495"_name,   34_position,   1_bit, {} }, // Missiles @  4,21
        { "taken_msl_24BB"_name,   39_position,   1_bit, {} }, // Missiles @  5,27
        { "taken_msl_2559"_name,   40_position,   1_bit, {} }, // Missiles @ 10,25
        { "taken_msl_2658"_name,   43_position,   1_bit, {} }, // Missiles @ 18,24
        { "taken_msl_269E"_name,   46_position,   1_bit, {} }, // Missiles @ 20,30
        { "taken_msl_271B"_name,   49_position,   1_bit, {} }, // Missiles @ 24,27

        // Energy tank pickups. Each of these adds +1 energy tank, but the number caps at 6.
        { "taken_tnk_2327"_name,    4_position,   1_bit, {} }, // Energy Tank @ 25, 7
        { "taken_tnk_2363"_name,    9_position,   1_bit, {} }, // Energy Tank @ 27, 3
        { "taken_tnk_212E"_name,   12_position,   1_bit, {} }, // Energy Tank @  9,14
        { "taken_tnk_2353"_name,   30_position,   1_bit, {} }, // Energy Tank @ 26,19
        { "taken_tnk_2156"_name,   36_position,   1_bit, {} }, // Energy Tank @ 10,22
        { "taken_tnk_211D"_name,   42_position,   1_bit, {} }, // Energy Tank @  8,29 (Kraid’s room)
        { "taken_tnk_2239"_name,   45_position,   1_bit, {} }, // Energy Tank @ 17,25
        { "taken_tnk_21FD"_name,   48_position,   1_bit, {} }, // Energy Tank @ 15,29

        // Zebetites in Mother Brain’s room.
        { "zeb_killed1_3C00"_name, 53_position,   1_bit, {} }, // 1st Zebetite @ 4,11 ◆
        { "zeb_killed2_4000"_name, 54_position,   1_bit, {} }, // 2nd Zebetite @ 4,11 ◆
        { "zeb_killed3_4400"_name, 55_position,   1_bit, {} }, // 3rd Zebetite @ 3,11 ◆
        { "zeb_killed4_4800"_name, 56_position,   1_bit, {} }, // 4th Zebetite @ 3,11 ◆
        { "zeb_killed5_4C00"_name, 57_position,   1_bit, {} }, // 5th Zebetite @ 2,11 ◆

        // Mother brain killed. ◆ This bit is always zero in passwords generated by the game.
        // This is because if you die in the game after killing mother brain, the game
        // punishes you by resetting not only this bit, but also all five zeb_killed flags
        // and also the last two missile doors in Tourian: door_open_2927 and door_open_292B.
        // The resetting happens only in password data; RAM contents are not cleared.
        { "motherbrain_3800"_name, 58_position,   1_bit, {} }, // Mother Brain  @ 2,11

        // The preceding 59 bits are stored in an unsorted array at $6887 ending at $68FC,
        // called “item history”. $6886 indicates the number of items in the history.
        // Each item in the array is identified by a 16-bit number (the hex code in the name).

        // Ridley and Kraid sub-boss states.
        // A nonzero value adds +75 to the maximum missile count.
        // Value >= 1 means boss is dead, and value >= 2 means statue at 4,2 is raised.
        { "ridley_state"_name,    124_position,  2_bits, {} }, // RAM address $687C bits #0 and #7
        { "kraid_state"_name,     126_position,  2_bits, {} }, // RAM address $687B bits #0 and #7

        // Number of missiles. May be greater than the maximum number of missiles.
        { "missiles"_name,         80_position,  8_bits, {} }, // RAM address $95

        // Age of Samus, as a 32-bit integer.
        // Unit: On NTSC systems, exactly 20965472/4921875 seconds.
        // Note: On PAL systems,    it is 8511360/1662607 seconds.
        { "age"_name,              88_position, 32_bits, {} }, // RAM address $687D-6880

        // Starting area.
        // Areas 0-4 are valid: Brinstar, Norfair, Kraid’s hideout, Tourian, Ridley’s hideout.
        // Values 5-15 are invalid, but do not lead to rejection of password.
        { "area_code"_name,        64_position,  4_bits, {} }, // RAM address $74 bits 0-3

        // The area code is technically 6-bit, but the last bit does not affect the game.
        // The second-to-last bit is here. The meaning of this bit is unknown,
        // but it seems to be set by the game when you land from an elevator.
        { "area_ext"_name,         68_position,   1_bit, {} }, // RAM address $74 bit 4

        // An unused bit that the game _does_ save.
        // If you load a game using a password that has this bit set,
        // the password generated by the game later also has this bit set.
        // However, entering an elevator will reset this bit.
        { "unused_bit08.5"_name,   69_position,   1_bit, {} }, // RAM address $74 bit 5

        // A total of ten unused bits that the game does not save anywhere.
        // These are always zero in passwords generated by the game, even
        // if you load a game using a password where these values are nonzero.
        // Setting these values does not affect the game in any manner.
        { "ghost_bit07.3"_name,    59_position,  5_bits, {} },
        { "ghost_bit08.6"_name,    70_position,   1_bit, {} },
        { "ghost_bit0F.0"_name,   120_position,  4_bits, {} },

        // NAR cheat mode. This flag is not part of the password. Rather, it is enabled
        // by loading a password that begins with “NARPASSWORD” followed by five zeros
        // or blanks. The rest of the password is ignored, including the checksum.
        // If this flag is set, the game is in NAR mode and gives you all gear
        // and sets high digit of HP as 3 on every frame. RAM address: $69B2
        { "nar_password"_name,    185_position, 1, {} },

        // This flag is not part of the password. It indicates the state of the checksum
        // in the last Decode()’d password. If the value is zero, or nar_password is set,
        // the password checksum was deemed OK. Otherwise, the password is to be rejected.
        // If this flag is set, Encode() will use the “sum” variable rather than recalculating
        // the checksum, thereby enabling lossless roundtrip of invalid passwords.
        { "password_error"_name,  184_position, 1, {} }
    }};

public:
    void Decode(const char* password)
    {
        // Decode symbols->letters->bytes
        std::array<std::uint64_t, 4> d{};
        for(unsigned n=0; n<24; ++n) { rotl(d.begin(),d.end(), 6); d[2] |= (CharSet.find(password[n]) & ones(6)) << 48;
                                       if(password[n] == ' ') d[2] |= ((n%4) ? 0xFFull : 0x3Full) << 48;
                                     }

        // Before decrypting the data, check for NARPASSWORD
        if(d[0] == MagicWords[0] && (d[1] >> 32) == MagicWords[1]) d[2] |= 2; // Set NAR flag

        // Decrypt and validate.
        rotl(d.begin(), std::next(d.begin(),2), d[2]>>56);
        if((checksum<16>(d[0]) + (d[2]>>56) - (d[2]>>48)) & ones(8)) d[2] |= 1; // Set fail flag

        // Extract data.
        for(auto& v: d) v = byteswap(v);
        for(auto& [name,bitpos,n_bits,value]: data) { value = (d[bitpos/64] >> (bitpos%64)) & ones(n_bits); }
    }
    void Encode(char* password) const
    {
        // Place data
        std::array<std::uint64_t, 4> d{};
        for(auto& [name,bitpos,n_bits,value]: data) { d[bitpos/64] |= (value & ones(n_bits)) << (bitpos%64); }
        for(auto& v: d) v = byteswap(v);

        // Encrypt. Recalculate checksum unless it’s requested to be not done (by the fail flag).
        if(!(d[2] & 1)) { d[2] &= ~(ones(8)<<48); d[2] |= (((d[2]>>56) + checksum<16>(d)) & ones(8)) << 48; }
        rotl(d.begin(), std::next(d.begin(),2), -(d[2]>>56));

        // If NAR cheat flag was enabled, replace the first 12 bytes / 16 characters.
        if(d[2] & 2)    { d[0] = MagicWords[0]; d[1] = (d[1]<<32>>32) | (MagicWords[1]<<32); }

        // Encode bytes->letters->symbols
        for(unsigned n=0; n<24; ++n) { rotl(d.begin(),d.end(), 6); password[n] = CharSet[d[3] & ones(6)]; }
    }

    void Describe(const char* password) const
    {
        // Print the password
        std::string test = password;
        for(std::size_t p=0; p<test.size(); ++p)
            if(test[p] == ' ')
                test.replace(p, 1, (const char*) u8"⎽", 0, 3);
        for(std::size_t p=test.size(); p--; )
        {
            if(test[p] != '0') break;
            test[p] = ' ';
        }
        std::printf("\33[97m%s ", test.c_str());
        unsigned doors=0, missiles=0, mismax=0, tanks=0, zebs=0;
        switch(std::get<3>(data[74])) // area
        {
            case 0: std::printf("Brinstar "); break;
            case 1: std::printf("Norfair  "); break;
            case 2: std::printf("Kraid’s  "); break;
            case 3: std::printf("Tourian  "); break;
            case 4: std::printf("Ridley’s "); break;
            default: std::printf("Area%02d ", (int)std::get<3>(data[74]));
        }
        for(auto [n,p,b,v]: data)
        {
            if(n == "missiles") { missiles=v; continue; }
            if(n == "sum" || n == "seed") continue;
            if(n == "area_code" || n == "area_ext") continue;
            if(v && n.compare(0,9,"door_open")==0) { ++doors; continue; }
            if(v && n.compare(0,9,"taken_msl")==0) { mismax += 5; continue; }
            if(v && n.compare(0,9,"taken_tnk")==0) { ++tanks; continue; }
            if(v && n.compare(0,9,"zeb_kille")==0) { ++zebs; continue; }
            if(v && n.compare(0,6,"taken_")==0) continue;
            if(n.find("_bit0") != n.npos) continue;
            switch(p)
            {
                case 72: { const char* item = "Bomb"; std::printf("%-5s", v?item:""); } continue;
                case 73: { const char* item = "HJmp"; std::printf("%-5s", v?item:""); } continue;
                case 74: { const char* item = "Long"; std::printf("%-5s", v?item:""); } continue;
                case 75: { const char* item = "Scrw"; std::printf("%-8s", v?item:""); } continue;
                case 76: { const char* item = "Maru"; std::printf("%-8s", v?item:""); } continue;
                case 77: { const char* item = "Vari"; std::printf("%-5s", v?item:""); } continue;
                case 78: { const char* item = "WBea"; std::printf("%-5s", v?item:""); } continue;
                case 79: { const char* item = "IBea"; std::printf("%-5s", v?item:""); } continue;
                case 71: { const char* item = "Leotard"; std::printf("%-7s", v?item:""); } continue;
                case 58: if(!v) continue; std::printf("\33[38;2;255;100;100m Mother Brain killed"); continue;
                case 124: if(!v) continue; std::printf("\33[38;2;255;100;100m Ridley %s", v==1?"killed":"statue up"); continue;
                case 126: if(!v) continue; std::printf("\33[38;2;255;100;100m Kraid %s", v==1?"killed":"statue up"); continue;
                case 88:
                {
                    if(!v) continue;
                    unsigned age = v;
                    if((age & 0xFF) == 0xFF)      age = (age >> 8) * 208 - 1;
                    else if((age & 0xFF) >= 0xD0) age = (age >> 8) * 208 + 207;
                    else                          age = (age >> 8) * 208 + (age & 0xFF);
                    double s = age * (256*655171/39375000.);
                    unsigned d = s/86400;
                    std::printf("\33[38;2;50;160;255m");
                    if(d)
                    {
                        unsigned y = d / 365.24219;
                        unsigned dd = d - y * 365.24219;
                        if(y) std::printf(" %dy", y);
                        std::printf(" %dd", dd);
                        s -= d*86400.;
                    }
                    unsigned h = s/3600;  if(h)   { std::printf(" %dh", h); s -= h*3600.; }
                    unsigned min = s/60;  if(min) { std::printf(" %dmin", min); s -= min*60.; }
                    std::printf(" %.1fs age", s);
                    continue;
                }
            }
            if(v==0) continue;
            std::printf(" %s=%*ld", n.data(), (b*100+332)/333, v);
        }
        if(doors) std::printf(" \33[92m\33[97m%u doors", doors);
        if(missiles || mismax) std::printf(" \33[92m%u\33[97m/\33[92m%u\33[97m missiles", missiles,mismax);
        if(tanks) std::printf(" \33[92m%u\33[97m tanks", tanks);
        if(zebs) std::printf(" \33[92m%u\33[38;2;255;100;100m zebetites killed", zebs);
        std::printf("\n");
    }
};

#include <iostream>
int main()
{
    /**/
    MetroPass tmp;
    char test[32]={};

    tmp.Decode("JUSTINBAILEY------------"); tmp.Encode(test); std::cout << test << '\n';
    tmp.Decode("NARPASSWORD00000-BISQWIT"); tmp.Encode(test); std::cout << test << '\n';
    tmp.Decode("ODDISHTAUROSMEWTWOVULPIX"); tmp.Encode(test); std::cout << test << '\n';
    tmp.Decode("X-------N?WOdV-Gm9W01GMI"); tmp.Encode(test); std::cout << test << '\n';
}
