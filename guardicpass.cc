#include <cstdint>
#include <algorithm>
// The Guardian Legend password generator (C) 2016 Joel Yliluoma http://iki.fi/bisqwit/ Written for: https://youtu.be/P3KD5TA8Tdk
static constexpr char set[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123abcdefghijklmnopqrstuvwxyz456789!?";
struct Guardian
{
    std::uint64_t bits{}/*bitmask*/;
    unsigned bits2{}/*bitmask*/, score{}/*0..999999*/, corridors{}/*bitmask*/;
    unsigned char weapons[12]{}/*0..3, except for weapons[11] which is 0..1 */, keys{}/*bitmask*/;
    unsigned char ammo{}/*0..255*/, maxlife{}/*1..32*/, firerate{}/*0..5*/, shield{}/*1..7 or 255*/;
    unsigned char attack{}/*1..4*/, coord[2]{}, chiplevel{}/*0..15*/;
public:
    bool Decode(const char *password)
    {
        // sym() converts alphabetical password into numbers.
        auto sym = [=](unsigned n) { auto p = password; while(*p && (*p==' ' || n--)) ++p; return std::find(set,set+64,*p)-set; };
        // get() does BASE64 decoding, b() returns decrypted bytes.
        auto get = [sym](unsigned n) { return (sym(n/3*4+n%3) >> (n%3*2)) | (sym(n/3*4+n%3+1) << (6-n%3*2)); };
        auto b   = [get](unsigned n) { return std::uint64_t(get(n) ^ (n!=23 ? get(23) + 29 - n : 0)) & 0xFFu; };
        // Extract data
        bits  = b(0) + (b(1)<<8) + (b(2)<<16) + (b(3)<<24) + (b(4)<<32) + (b(5)<<40) + (b(6)<<48) + (b(7)<<56);
        bits2 = b(8) & 15;
        corridors = b(9) + (b(10) << 8) + ((b(11) & 0xF0) << 12);
        for(unsigned n=0; n<12; ++n) weapons[n] = (b(12+n/4) >> (n%4*2)) & (n==11 ? 1 : 3);
        ammo = b(15);
        keys = b(16) & 0x7F; // The game actually clears bit 7 of keys when decoding the password.
        firerate  = Rev<3>(b(17) & 7); maxlife  = b(17) >> 3; if(!maxlife) maxlife=32;
        attack    = Rev<3>(b(18) & 7); coord[0] = b(18) >> 3; ++attack;
        shield    = Rev<3>(b(19) & 7); coord[1] = b(19) >> 3; if(!shield) shield=255;
        chiplevel = Rev<4>(b(8) >> 4);
        score = (b(21) | (Rev<2>(b(11)     ) << 8))
              + (b(20) | (Rev<2>(b(11) >> 2) << 8))*1000;
        // Verify checksum and password alphabet validity
        unsigned syms=0, sum=0;
        for(unsigned n=32; n-- > 0; syms |= sym(n)) if(n<22) sum += b(n) + sum/256%2*257;
        // Note: In the video I derped. The checksum calculation should be downwards from 21 to 0, and last 23.
        return syms < 64 && b(22) == ((sum + b(23) + sum/256%2*257) & 0xFF);
    }
    void Encode(char* password, unsigned key) const
    {
        // Populate the password bytes with data
        auto b = [=](unsigned n) { return std::uint8_t(bits >> n); };
        auto w = [](const unsigned char* w) { return w[0]%4u + w[1]%4u*4u + w[2]%4u*16u + w[3]%4u*64u; };
        unsigned scorelo=score%1000, scorehi=score/1000, bytes[24] {
            b(0),b(8),b(16),b(24),b(32),b(40),b(48),b(56), (bits2&15) + 16*Rev<4>(chiplevel),
            corridors, corridors>>8, ((corridors>>12)&0xF0) + Rev<2>(scorelo>>8) + 4*Rev<2>(scorehi>>8),
            w(weapons+0), w(weapons+4), w(weapons+8), // Game does w(weapons+8)&0x7F, but it's not necessary
            ammo,keys, Rev<3>(std::min(7,int(firerate))) + 8*(std::min(32,int(maxlife))&31),
            Rev<3>((std::min(int(attack),8)-1)&7) + 8*coord[0], Rev<3>(shield>7?0:shield) + 8*coord[1],
            scorehi,scorelo, 0, key };
        // Calculate and insert checksum
        for(unsigned n=22; n-- > 0; ) bytes[22] += (bytes[n] & 0xFF) + bytes[22]/256%2*257;
        bytes[22] += (bytes[23] & 0xFF) + bytes[22]/256%2*257;
        // byt() returns encrypted bytes.
        auto byt = [&](unsigned n) -> std::uint8_t { return bytes[n] ^ (n!=23 ? bytes[23]+29-n : 0u); };
        // BASE64-encode the encrypted bytes using the password alphabet.
        for(unsigned n=0; n<32; ++n)
            password[n] = set[((byt(n/4*3+(n%4)%3) << (n%4*2)) | (byt(n/4*3+(n%4+2)%3) >> (8-n%4*2))) & 0x3F];
        password[32] = '\0';
    }
    template<unsigned N> static unsigned Rev(unsigned v) // Reverse bits for N-bit integer where N=0..4
        { v = ((v&3)*4 + (v&12)/4); return ((v&10)/2 + (v&5)*2) >> (4-N); }
};

#include <utility>
#include <cstdio>
static std::pair<int,int> getpos(char c)
{
    switch(c)
    {
        case 'A'...'J': return {0,c-'A'};
        case 'K'...'T': return {1,c-'K'};
        case 'U'...'Z': return {2,c-'U'}; case '0'...'3': return {2,c-'0'+6};
        case 'a'...'j': return {3,c-'a'};
        case 'k'...'t': return {4,c-'k'};
        case 'u'...'z': return {5,c-'u'}; case '4'...'7': return {5,c-'4'+6};
        case '8': return {6,0};
        case '9': return {6,1};
        case '!': return {6,2};
        case '?': return {6,3};
    }
    return {15,15};
}
template<typename T>
static unsigned distance(std::pair<T,T> pos1, std::pair<T,T> pos2)
{
    // Returns toroidical manhattan distance between the two points in password symbol grid
    return 2*std::min(std::abs(pos1.second-pos2.second),  T(7)-std::abs(pos1.second-pos2.second))
         + 1*std::min(std::abs(pos1.first -pos2.first ), T(10)-std::abs(pos1.first -pos2.first ));
}

int main(int argc, char** argv)
{
    Guardian test;

    if(argc > 1)
    {
        std::printf("Decoding of <%s>: %s\n", argv[1], test.Decode(argv[1]) ? "successful":"failure");
        std::printf("Bits=%X%016llX Corridors=%05X Weapons=%u%u%u%u%u%u%u%u%u%u%u%u ammo=%u keys=%02X score=%06u0 maxlife=%u fire=%u shield=%u attack=%u chip=%u coord=%u,%u\n",
            test.bits2, (unsigned long long)(test.bits), test.corridors,
            test.weapons[0],test.weapons[1],test.weapons[2],test.weapons[3],test.weapons[4],test.weapons[5],
            test.weapons[6],test.weapons[7],test.weapons[8],test.weapons[9],test.weapons[10],test.weapons[11],test.ammo,test.keys,
            test.score, test.maxlife,test.firerate,test.shield,test.attack, test.chiplevel, test.coord[0],test.coord[1]);
    }

    // Create a password
    test = Guardian{};
    test.bits      = ~0ull;
    test.bits2     = ~0u;
    test.corridors = ~0u;
    for(unsigned n=0; n<12; ++n) test.weapons[n]=3;
    test.maxlife=32; test.firerate=5; test.shield=0; test.attack=0xFF; test.chiplevel=0xFF;
    test.ammo=0xFF; test.keys=0xFF;
    test.coord[0]=7; test.coord[1]=0;

    // Attempt to find the password that is easiest to type,
    // by adjusting the unused bits and the score.
    // Note: This will take a long time! Compile with -fopenmp to speed it up (still takes time).
    struct besttype { unsigned m=0,k=0,pen=~0u; } best;
    #pragma omp declare reduction(isbetter:besttype: (omp_in.pen<omp_out.pen || omp_out.pen==~0u) ? omp_out=omp_in : omp_out)
    #pragma omp parallel for collapse(2) firstprivate(test) reduction(isbetter:best) default(none)
    for(unsigned k=0; k<1024; ++k)
    for(unsigned m=0; m<=999999; m+=1)
    {
        test.score = m;
        test.weapons[11] = (test.weapons[11] & 0x01) | (((k>>8)&1)*0x02);
        test.keys        = (test.keys        & 0x7F) | (((k>>9)&1)*0x80);
        char pass[33]; test.Encode(pass, k & 0xFF);
        unsigned penalty = 0; auto p = getpos(pass[0]);
        for(unsigned n=1; n<32; ++n) { auto p2 = getpos(pass[n]); penalty += distance(p, p2); p=p2; }

        if(penalty < best.pen || best.pen==~0u)
        //if(penalty > best[2] || best[2]==~0u)
        {
            best = {m,k,penalty};
            //std::printf("Encoded as: %s with key %u, money %u, penalty %u\n", pass, k, m, penalty);
        }
    }
    test.score   = best.m;
    unsigned key = best.k;
    test.weapons[11] = (test.weapons[11] & 0x01) | (((key>>8)&1)*0x02);
    test.keys        = (test.keys        & 0x7F) | (((key>>9)&1)*0x80);

    char pass_char[33];
    test.Encode(pass_char, key&0xFF);
    std::printf("Encoded as: <%.4s %.4s %.4s %.4s %.4s %.4s %.4s %.4s>   with key: %u score %u penalty %u\n",
        pass_char+0,pass_char+4,pass_char+8,pass_char+12,
        pass_char+16,pass_char+20,pass_char+24,pass_char+28,
        key, test.score, best.pen);

    test = Guardian{};
    std::printf("Decoding of <%s>: %s\n", pass_char, test.Decode(pass_char) ? "successful":"failure");
    std::printf("Bits=%X%016llX Corridors=%05X Weapons=%u%u%u%u%u%u%u%u%u%u%u%u ammo=%u keys=%02X score=%06u0 maxlife=%u fire=%u shield=%u attack=%u chip=%u coord=%u,%u\n",
        test.bits2, (unsigned long long)(test.bits), test.corridors,
        test.weapons[0],test.weapons[1],test.weapons[2],test.weapons[3],test.weapons[4],test.weapons[5],
        test.weapons[6],test.weapons[7],test.weapons[8],test.weapons[9],test.weapons[10],test.weapons[11],test.ammo,test.keys,
        test.score, test.maxlife,test.firerate,test.shield,test.attack, test.chiplevel, test.coord[0],test.coord[1]);
}
