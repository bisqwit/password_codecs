struct Bomberman
{
    unsigned      score{}, stage{};
    unsigned char p_detonator, p_range, p_flameimmunity, p_numbombs, p_speedup, p_wallwalk, p_radar;
public:
    #define have(n,op,s,c)  c(n[ 4], s(n[ 0],sc[0]) + s(n[ 1],p_detonator) + s(n[ 2],lv[0]) + s(n[ 3],sc[6])) \
                         op c(n[ 9], s(n[ 5],sc[1]) + s(n[ 6],p_range)     + s(n[ 7],sc[3]) + s(n[ 8],p_flameimmunity)) \
                         op c(n[14], s(n[11],sc[4]) + s(n[12],p_speedup)   + s(n[13],sc[5]) + s(n[10],p_numbombs)) \
                         op c(n[19], s(n[15],sc[2]) + s(n[18],p_wallwalk)  + s(n[17],lv[1]) + s(n[16],p_radar) + 2*(n[4]+n[9]+n[14]))
    bool Decode(const char *password)
    {
        unsigned char nib[20], sc[7], lv[2];
        // Decipher and deconvolute
        for(unsigned tmp, prev=0, k=0; k<20; ++k)
            { tmp = "094m726zol38kn15"[password[k]-'A']; nib[k] = (tmp + (prev + 7)) % 16; prev = tmp; }
        // Extract data; return true if all checksums matched.
        bool ok = have(nib,&&, [](int nib,auto& val) { return val=nib; }, [](int nib,int chk) { return nib==chk%16; });
        // Recompose score & stage number
        score = sc[0]*1 + sc[1]*10 + sc[2]*100 + sc[3]*1000 + sc[4]*10000 + sc[5]*100000 + sc[6]*1000000;
        stage = lv[0]*1 + lv[1]*16;
        return ok;
    }
    void Encode(char *password) const
    {
        unsigned char nib[20], sc[7], lv[2];
        // Decompose score & stage number
        for(unsigned p=1, k=0; k<7; ++k, p*=10) sc[k] = (score/p) % 10; // Score digits
        for(unsigned p=1, k=0; k<2; ++k, p*=16) lv[k] = (stage/p) % 16; // Stage nibbles
        // Insert data
        have(nib,;, [](auto& nib,int val) { return nib=val; }, [](auto& nib,int chk) { nib=chk; });
        // Convolute and encipher
        for(unsigned prev=0, k=0; k<20; ++k)
            password[k] = "AOFKCPGELBHMJDNI"[ prev = (nib[k] - (prev + 7)) % 16 ];
    }
    #undef have
};

#include <string>
#include <cstdio>
#include <cstring>
#include <algorithm>
int main(int argc, char** argv)
{
    Bomberman test;
    char password[21]{};
    std::strncpy(password, argv[1], 20);

    std::printf("Decoding of %s: %s\n", password, test.Decode(password)?"successful":"failure");
    std::printf("Score: %08u, stage: %03u, powerups: remote=%d range=%d inv=%d nbombs=%d speed=%d intang=%d cheat=%d\n",
        test.score, test.stage, test.p_detonator, test.p_range, test.p_flameimmunity,
        test.p_numbombs, test.p_speedup, test.p_wallwalk, test.p_radar);

    unsigned bestsum=~0u, best=0;
    for(unsigned score=0; score<10000000; ++score)
    {
        test.score = score;
        test.stage = 197;          // range: 1-50. 0 works mostly fine except the game crashes if you bomb the exit.
        test.p_range=15;           // works. 0 is treated as 1.
        test.p_numbombs=9;         //

        test.p_detonator=1;       // 1 and 8 work
        test.p_flameimmunity=1;   // ?
        test.p_speedup=1;         // walk faster
        test.p_wallwalk=1;        // pass through walls (works!)
        test.p_radar=1;           // shows tiles hiding something behind them

        test.Encode(password);
        unsigned sum=0;
        for(unsigned k=0; k<20; ++k) sum += std::min(password[k]-'A', 'P'-password[k]);
        if(sum < bestsum) { bestsum=sum; best=score; }
    }
    test.score = best;
    test.Encode(password);
    std::printf("Encoded as: %s\n", password);
    std::printf("Decoding of %s: %s\n", password, test.Decode(password)?"successful":"failure");
    std::printf("Score: %08u, stage: %03u, powerups: remote=%d range=%d inv=%d nbombs=%d speed=%d intang=%d cheat=%d\n",
        test.score, test.stage, test.p_detonator, test.p_range, test.p_flameimmunity,
        test.p_numbombs, test.p_speedup, test.p_wallwalk, test.p_radar);
}
