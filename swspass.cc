#include <algorithm>
#include <cstdio>
#include <string>
#include <tuple>
#include <array>
static constexpr char cset[32+1] = "?234679ABCEFGHIJKLMNOPQRSTUVWXYZ";

class SwordsSerpents
{
    class Password
    {
        template<typename T=unsigned> // Calculate (1u << n) - 1 without invoking UB when n = 32
        static T Mask(unsigned n) { T o{1}; return n ? (((o << (n-1))-o) << 1) + o : T{}; }

        template<unsigned N, typename T=unsigned>
        static unsigned Rev(T v) // Reverse bits for N-bit integer of type T, where N = 0..M, in log(N) operations,
        {                        // where M = std::numeric_digits<T>::digits * std::numeric_digits<T>::radix
            if(N>2 && !((N-1) & (N-2))) // Optional optimization when popcnt(N/2)*2 <= popcnt(N), approx. by N=2^x+1
            {
                constexpr T Half { N/2 }, M{ T(1) << Half }, MM{ M << 1 };
                return (v & M) + Rev<Half>(Half%2 ? v%M : v)*MM + Rev<Half>(v/MM%M);
            }
            T cap{1}, max{1};
            for(unsigned b = 1; b < N; ) { b <<= 1; max = Mask<T>(b); }
            for(unsigned b = 1; b < N; cap += b, b <<= 1)
            {
                const T mask = max / (Mask<T>(b) + 2); // Repeating pattern of 1010 where period=b*2
                v = ((v & mask) << b) + ((v >> b) & mask);
            }
            return v >> (cap-N);
        }

        std::uint64_t l{}, h{}; // Ideally we'd have 128-bit uint, but C++ doesn't support it yet.
    public:
        Password(const std::string& s = {}) // Convert string into integer (empty string is ok)
        {
            for(auto r = s.rbegin(); r != s.rend(); ++r)
                if(*r != ' ')
                    PutBits<5>(std::find(cset, cset+32, *r) - cset);
        }
        operator std::string() && // Convert the integer into a string (destructively)
        {
            char Buf[128/5+1], *end=Buf;
            while(h || l) *end++ = cset[GetBits<5>()];
            return {Buf, end};
        }
        template<unsigned N>
        void PutBits(unsigned val) // Insert bits into the 128-bit integer
        {
            h = (h << N) | (l >> (64-N));
            l = (l << N) | Rev<N>(val & Mask(N));
        }
        template<unsigned N>
        unsigned GetBits(unsigned crypt=0) // Extract bits from the 128-bit integer
        {
            unsigned res = l; // take low bits
            l = (l >> N) | (h << (64-N));
            h = (h >> N);
            return (Rev<N>(res) ^ crypt) & Mask(N); // Return decrypted value (crypt=0 = no changes)
        }
        unsigned CheckSum3() const // Return the 3-bit checksum of all bytes within the 128-bit integer
        {
            std::uint64_t m = ~0ull/255*7, a = (l&m) + (h&m), b = a + (a>>32), c = b + (b>>16), d = c + (c>>8);
            return d & Mask(3);
        }
    };
public:
    struct Character
    {
        std::string name;
        unsigned char profession, strength, intelligence, agility, maxhp, inventory[6]{};
        unsigned spells;
    } chars[4];
    unsigned char level, map,coordX,coordY, itty,bitty;
    unsigned treasury, flags;

    // Decode a set of passwords.
    template<typename ReportError>
    void Decode(const std::array<std::string,5>& words, ReportError&& report)
    {
        unsigned cryptsum=0;
        // Decode the four character-passwords (in any order)
        for(unsigned n=0; n<4; ++n)
        {
            auto& ch = chars[n];
            unsigned crypt = NameChecksum(n); cryptsum += crypt;
            Password pass{ words[n] };
            // First load and verify checksum
            unsigned chk = pass.GetBits<3>(crypt), sum = pass.CheckSum3();
            if(chk != sum) report("Character %u: checksum failure, got %u expected %u", n, chk, sum);
            // Load stats
            ch.strength     = pass.GetBits<5>(crypt);
            ch.intelligence = pass.GetBits<5>(crypt);
            ch.agility      = pass.GetBits<5>(crypt);
            ch.maxhp        = pass.GetBits<5>(crypt);
            // Load inventory
            std::fill_n(ch.inventory, 6, 0xFF);
            for(unsigned p=0, u = pass.GetBits<3>(crypt); p < u; ++p)
            {
                unsigned i = pass.GetBits<7>(crypt), flag = (i&0x40)<<1; i &= 0x3F;
                if(p < 6) ch.inventory[p] = i|flag;
                else ch.spells = (ch.spells & 0xFF00) | i|flag; // handle overflow to byte 6
                if((1ull << i) & 0xE3F1C04000000000ull) // Bitmask of items that are prohibited in inventory
                    report("Character %u inventory item %u: %02X is not allowed", n, p, i);
            }
            // Load profession
            ch.profession = pass.GetBits<2>(crypt);
            // Verify limits
            if(ch.strength     >= 21) report("Character %u strength %u exceeds limit 20",     n, ch.strength);
            if(ch.intelligence >= 21) report("Character %u intelligence %u exceeds limit 20", n, ch.intelligence);
            if(ch.agility      >= 21) report("Character %u agility %u exceeds limit 20",      n, ch.agility);
            // Spell points are set to min(intelligence + maxhp-10, 31) if magician, 0 otherwise.
        }
        // Decode the game password
        Password pass{ words[4] };
        // Start by verifying the checksum
        unsigned chk = pass.GetBits<3>(cryptsum), sum = pass.CheckSum3();
        if(chk != sum) report("Gamestate checksum failure: got %u, expected %u", chk, sum);
        // If there is a magician in the party, load the list of spells
        if(!chars[0].profession || !chars[1].profession || !chars[2].profession || !chars[3].profession)
        {
            // Read these two bitmasks, and assign them to all magicians; everyone else gets 0.
            unsigned E3 = pass.GetBits<8>(cryptsum), E2 = pass.GetBits<6>(cryptsum)*4 + 3;
            for(auto& ch: chars) ch.spells = ch.profession ? 0 : (E2 | (E3 << 8));
        }
        // Read the two unknown bits
        bitty = pass.GetBits<1>() ? 255 : 0;
        itty  = pass.GetBits<1>() ? 255 : 0; // no xorring here
        // Read the money collection and enemy beating flags in various stages
        for(unsigned n=26; n-- > 0; ) flags = (flags & ~(1u<<n)) | ((pass.GetBits<1>() != (n==1 || n>=23)) << n);
        // Read level
        level = pass.GetBits<5>(cryptsum);
        // Decide which temple the player starts in: map,x,y.
        typedef std::tuple<int,int,int> t; t locations[4]{ t{0,9,9}, t{4,14,14}, t{9,5,10}, t{20,2,7} };
        std::tie(map, coordX, coordY) = locations[pass.GetBits<2>(cryptsum)];
        // Read treasury
        unsigned hi = pass.GetBits<6>(cryptsum), lo = pass.GetBits<8>(cryptsum);
        treasury = (hi << 8) | lo;
        // Exp is cleared
    }

    // Encode the state into a set of passwords.
    std::array<std::string,5> Encode() const
    {
        unsigned cryptsum=0;
        // Generate the per-character passwords (in any order)
        std::array<Password,4> cp;
        for(unsigned n=0; n<4; ++n)
        {
            const auto& ch = chars[n];
            unsigned crypt = NameChecksum(n); cryptsum += crypt;
            // Put profession
            cp[n].PutBits<2>(crypt ^ ch.profession);
            // Put inventory. Item numbers are 00..3F. Bitmask &80 is saved as bitmask &40.
            unsigned nitems = 0;
            for(unsigned p=0; p<6; ++p)
                if(ch.inventory[p] != 0xFF)
                    { cp[n].PutBits<7>(crypt ^ (ch.inventory[p]%64 + ch.inventory[p]/128*64)); ++nitems; }
            cp[n].PutBits<3>(crypt ^ nitems);
            // Put stats. Make sure they are in valid range, so that the password decodes properly.
            cp[n].PutBits<5>(crypt ^ std::min(31,int(ch.maxhp))       );
            cp[n].PutBits<5>(crypt ^ std::min(20,int(ch.agility))     );
            cp[n].PutBits<5>(crypt ^ std::min(20,int(ch.intelligence)));
            cp[n].PutBits<5>(crypt ^ std::min(20,int(ch.strength))    );
            // Put checksum
            cp[n].PutBits<3>(crypt ^ cp[n].CheckSum3());
        }
        // Generate the game password
        Password gp;
        // Put treasury
        gp.PutBits<8>(cryptsum ^ (treasury     ));
        gp.PutBits<6>(cryptsum ^ (treasury >> 8));
        // Put coordinates (only save highest floor where temple has been reached)
        gp.PutBits<2>(cryptsum ^ (map<4 ? 0 : map<9 ? 1 : 2));
        // Put level
        gp.PutBits<5>(cryptsum ^ level);
        // Put money collection and enemy beating flags in various stages
        for(unsigned n=0; n<26; ++n) gp.PutBits<1>( ((flags >> n) & 1) ^ (n==1 || n>=23));
        // Put these two unknown bits
        gp.PutBits<1>(itty  & 1);
        gp.PutBits<1>(bitty & 1); // no xorring
        // If there is a magician in the party, save the list of spells the first one has
        for(const auto& ch: chars)
            if(ch.profession == 0)
            {
                gp.PutBits<6>(cryptsum ^ (ch.spells >> 2));
                gp.PutBits<8>(cryptsum ^ (ch.spells >> 8));
                break;
            }
        // Put checksum
        gp.PutBits<3>(cryptsum ^ gp.CheckSum3());
        return {{ std::move(cp[0]), std::move(cp[1]), std::move(cp[2]), std::move(cp[3]), std::move(gp) }};
    }

    // Calculate the checksum of the name of character n.
    unsigned NameChecksum(unsigned n) const
    {
        auto l = [s = chars[n].name](unsigned i) { return (i < s.size()) ? s[i] : ' '; };
        return l(0) + l(1) + l(2) + l(3) + l(4) + l(5);
    }

    // Set the names for all characters.
    void SetNames(const std::array<std::string,4>& names)
    {
        for(unsigned n=0; n<4; ++n) chars[n].name = names[n];
    }
};

void Dump(const SwordsSerpents& s)
{
    static const char professions[4][9] = {"magician","warrior","thief","(thief)"};
    for(unsigned n=0; n<4; ++n, std::putchar('\n'))
    {
        std::printf("Character %u (%.6s): prof=%-8s str=%2u int=%2u agi=%2u hp=%2u spells=$%04X magic=%2u inv:",
            n, s.chars[n].name.c_str(), professions[s.chars[n].profession&3],
            s.chars[n].strength, s.chars[n].intelligence, s.chars[n].agility, s.chars[n].maxhp, s.chars[n].spells,
            (s.chars[n].profession ? 0 : std::min(31, s.chars[n].maxhp-10+s.chars[n].intelligence)));
        for(unsigned m=0; m<6; ++m)
            if(s.chars[n].inventory[m] == 0xFF)
                std::printf(" -");
            else
                std::printf(" $%02X", s.chars[n].inventory[m]);
    }
    std::printf("Treasury %5u Level %2u exp=0 map={%u at %u,%u} itty=%3u bitty=%3u flags=$%X\n",
        s.treasury, s.level, s.map, s.coordX, s.coordY, s.itty, s.bitty, s.flags);
}

int main(int argc, char** argv)
{
    std::array<std::string,9> w{{ "IAGO", "MASK", "AJAX", "ERIN", // default names and passwords
                                  "T7PLC6T7?", "GWO6V7CIK", "7CTXVWL3CE", "GS?OM72FK", "QJALS???3ZNUGK" }};
    for(int a=1; a<argc && a<=9; ++a) w[a-1] = argv[a]; // replace with args if provided

    std::printf("Decoding result:\n");
    SwordsSerpents test;
    test.SetNames( {{ w[0],w[1],w[2],w[3] }} );
    test.Decode( {{ w[4],w[5],w[6],w[7],w[8] }}, [](auto&& ...args) { std::printf(args...); std::putchar('\n'); } );
    Dump(test);
    // For a test, maximize each character's stats
    for(auto& c: test.chars)
    {
        c.strength = c.intelligence = c.agility = 20;
        c.maxhp = 31; c.spells = 0xFFFF;
    }
    c.treasury = 0xFFFF; c.level = 31; c.map = 0;
    auto p = test.Encode();
    std::printf("Encoded as:\n");
    for(unsigned n=0; n<5; ++n)
        std::printf("  %s\n", p[n].c_str());

    test.Decode(p, [](auto&& ...args) { std::printf(args...); std::putchar('\n'); } );
    Dump(test);
}



