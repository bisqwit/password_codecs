#include <algorithm>
#include <cstdio>
#include <vector>
#include <mutex>
#include <map>
static const signed char hash[] = {6,3,5,7,9, 0,1,4,2,8}, offs[] = {1,2,0,-1, 2,0,1,-1, 0,1,2,-1, -1,-1,-1,-1};

// 0: Major Circuit, Don Flamenco
// 1: World Circuit: Piston Honda
// 2: World Circuit Title Bout: Super Macho Man

int main()
{
    std::map<std::string, std::vector<unsigned long long>> stats;
    std::mutex lk;
    #pragma omp parallel for
    for(unsigned long long pass=0; pass <= 9999999999; ++pass)
    {
        // Convert password into an array of digits
        unsigned char p[10];
        { auto v=pass; for(unsigned n=10; n--; ) { p[n]=v%10; v/=10; } }
        // Undo the first round of encryption: Substitution cipher
        for(unsigned n=0; n<10; ++n) if( (signed char)(p[n] -= hash[n]) < 0 ) p[n] += 10;
        // Undo the second round of encryption: Bit rotation
        signed char roll = p[9] & 3;
        for(unsigned r = roll; r--; )
        {
            for(unsigned n=9; n>=1; --n)
                p[n] = (p[n] >> 1) | ((p[n-1] & 1) << 2);
            p[0] = (p[0] >> 1) | ((p[9] & 2) << 1);
        }
        // Combine the ten 3-bit units into five 6-bit units
        for(unsigned n=0; n<5; ++n) p[n] = (p[n*2] << 3) | p[n*2+1];
        // Data extraction and range checking
        unsigned r0 = (p[0] & 0xC) | (p[1] & 3); if(r0 >= 10) continue;
        unsigned r1 = (p[3] & 0xC) | (p[2] & 3); if(r1 >= 10) continue;
        unsigned r3 = (p[4] >> 4) & 3;
        unsigned r4 = (p[2] & 0xC) | (p[3] & 3); if(r4 >= 10) continue;
        unsigned r5 = (p[1] & 0xC) | (p[0] & 3); if(r5 >= 10) continue;
        unsigned r6 = (p[4] >> 2) & 3; if(roll != offs[r6*4 + r3]) continue; // This verifies roll and also ensures r6<3 and r3<3.
        // Checksum validation
        if(((r0+r1+r3+r4+r5) ^ 0xFF)
        != ((p[0] & 0x30) | ((p[3] & 0x30) << 2) | ((p[1] & 0x30) >> 2) | ((p[2] & 0x30) >> 4))) continue;
        // Verify that the number of wins is at least 3,
        // and that the number of KOs is not larger than the number of wins.
        // This check doesn't quite work right when number of wins < 10, but it's exactly what the game does.
        unsigned A = r0;
        if(A == 0) { A = r1; if(A < 3) continue; }
        if(A < r4 || (A == r4 && r1 < r5)) continue;

        char name[64]; std::sprintf(name, "win=%d%d lose=%d ko=%d%d circuit=%d", r0,r1, r3, r4,r5, r6);
        std::lock_guard<std::mutex> l(lk);
        stats[name].push_back(pass);
        if(stats.size()%16 == 0) std::fprintf(stderr, "%llu\r", pass);
    }
    std::fprintf(stderr, "\n");

    unsigned totalsets=0, totalpass=0;
    for(auto& k: stats)
    {
        std::printf("%s", k.first.c_str());
        std::sort(k.second.begin(), k.second.end());
        for(auto pass: k.second)
            std::printf(" %010llu", pass);
        std::printf("\n");
        totalsets += 1;
        totalpass += k.second.size();
    }
    std::printf("%u settings, %u passwords\n", totalsets, totalpass);
}
