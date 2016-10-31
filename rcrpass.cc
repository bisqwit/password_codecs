#include <cstdio>
#include <codecvt>
#include <locale>
#include <string>
#include <numeric>
#include <map>

static const char         regmap[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
static const std::u32string extmap = U"Á_Ć_É_Ǵ_Í_ḰĹḾŃÓṔ_ŔŚ_Ú_Ẃ_ÝŹá_ć_é_ǵ_í_ḱĺḿńóṕ_ŕś_ú_ẃ_ýź";
static unsigned Key(unsigned n) { return u"なぢふつとのぺねひばぴてぬぶぼどぱぢだびっどはつでとへはになどふのそ"[n] - u'そ'; }
static constexpr unsigned char pack[] = {12,15,4, 13,15,2, 14,15,0, 8,10,2, 9,10,0, 16,18,2, 17,18,0,
                                         19,21,2, 20,21,2, 22,24,2, 23,24,0, 25,27,2, 26,27,0, 28,30,2, 29,30,0};
struct RiverCityRansom
{
    unsigned Stats[11]{}, Money{}, Inventory[8]{}, Mask1{}, Mask2{}, MoneyHex{};
    bool Decode(const std::string& password)
    {
        // Convert the utf-8 character string into bytes
        unsigned char bytes[33]{}, pos=0;
        for(auto c: std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.from_bytes(password))
            switch(c)
            {
                case U'A' ... U'Z': if(pos < 33) bytes[pos++] = c-U'A' +  0; break;
                case U'a' ... U'z': if(pos < 33) bytes[pos++] = c-U'a' + 26; break;
                case U'0' ... U'9': if(pos < 33) bytes[pos++] = c-U'0' + 52; break;
                case char32_t(0x301): case U'\'': case char32_t(0x341): if(pos > 0) bytes[pos-1] += 62; break;
                case U' ':          if(pos < 33) bytes[pos++] = 0; break;
                case U'\n':         if(pos % 11) pos += 11-(pos%11); break;
                default: if(pos < 33) bytes[pos++] = extmap.find(c) + 62;
            }
        // Fail if not at least one byte is nonzero
        if(std::accumulate(bytes,bytes+33,0) == 0) return false;
        // Decrypt
        for(unsigned n=0; n<32; ++n) bytes[n] ^= Key(n) + bytes[32];
        // Verify checksum
        if(bytes[31] != (std::accumulate(bytes,bytes+31, 0) & 0x3F)) return false;
        // Verify ranges
        for(auto c: bytes) if(c >= 0x40) return false;
        // Unpack bytes
        for(const auto* p=pack; p<pack+sizeof(pack); p+=3) bytes[p[0]] |= ((bytes[p[1]] >> p[2]) & 3) << 6;
        // Verify ranges of some unpacked values
        if(bytes[8] >= 128 || bytes[9] >= 128 || bytes[12] > 0x99 || bytes[13] > 0x99 || bytes[14] > 9) return false;
        // Extract
        for(unsigned c=0; c<11; ++c) Stats[c] = bytes[c + (c==10)];
        Money    = (bytes[12]&0xF) + (bytes[12]>>4)*10 + (bytes[13]&0xF)*100 + (bytes[13]>>4)*1000 + bytes[14]*10000;
        MoneyHex = bytes[12] + (bytes[13] << 8) + (bytes[14] << 16);
        for(unsigned c=0; c<8; ++c) Inventory[c] = bytes[16 + c + (c/2)];
        Mask1   = bytes[28];
        Mask2   = bytes[29];
        return true;
    }
    std::string Encode(unsigned key)
    {
        unsigned char bytes[33]{};
        // Insert data
        for(unsigned c=0; c<11; ++c) bytes[c + (c==10)] = Stats[c];
        bytes[12] = (Money%10) + (Money/10%10)*16;
        bytes[13] = (Money/100%10) + (Money/1000%10)*16;
        bytes[14] = (Money/10000%10);
        /*bytes[12] = 0x7F;
        bytes[13] = 0x6B;
        bytes[14] = 0x05;*/
        for(unsigned c=0; c<8; ++c) bytes[16 + c + (c/2)] = Inventory[c];
        bytes[28] = Mask1;
        bytes[29] = Mask2;
        // Ensure range validity
        if(bytes[8] > 0x7F) bytes[8] = 0x7F;
        if(bytes[9] > 0x7F) bytes[9] = 0x7F;
        // Pack bytes
        for(const auto* p=pack; p<pack+sizeof(pack); p+=3) bytes[p[1]] |= ((bytes[p[0]] >> 6) << p[2]);
        // Make sure they are all in valid range
        for(auto& b: bytes) b &= 0x3F;
        // Put checksum
        bytes[31] = std::accumulate(bytes,bytes+31, 0) & 0x3F;
        // Test whether the encrypted password is actually possible to represent. If not, choose another key.
        auto Encodable = [&]()
            { for(unsigned n=0; n<32; ++n) if(((bytes[n] ^ (Key(n) + bytes[32])) & 0xFF) >= 114) return false; return true; };
        for(bytes[32] = key; bytes[32] >= 0x40 || !Encodable(); --bytes[32]) {}
        // Encrypt
        for(unsigned n=0; n<32; ++n) bytes[n] ^= Key(n) + bytes[32];
        // Convert into characters.
        std::u32string result;
        for(unsigned c: bytes)
            if(c < 62)                    { result += regmap[c]; }
            //else if(extmap[c-62] != U'_') { result += extmap[c-62]; }
            else                          { result += regmap[c-62]; result += char32_t(0x301); } // Use combining acute accent
        return std::wstring_convert<std::codecvt_utf8_utf16<char32_t>, char32_t>{}.to_bytes(result);
    }
};

static std::map<char,std::pair<int,int>> distmap;
static void build_distancemap()
{
    static const std::string rows[] = {"AaBbCcDdEeZz","FfGgHhIiJj","KkLlMmNnOo","PpQqRrSsTt","UuVvWwXxYy","0123456789"};
    auto find = [&](char c) -> std::pair<int,int>
        { for(unsigned r=0; r<6; ++r) { auto p=rows[r].find(c); if(p!=rows[r].npos) return {r,p}; } return {15,15}; };
    for(char c: regmap) distmap[c] = find(c);
}

static unsigned distance(char a, char b)
{
    std::pair<int,int> pos1{15,15}, pos2{15,15};
    { auto i = distmap.find(a); if(i != distmap.end()) pos1 = i->second; }
    { auto i = distmap.find(b); if(i != distmap.end()) pos2 = i->second; }
    unsigned ydiff  = std::abs(pos1.second-pos2.second);
    unsigned xdiff1 = std::abs(pos1.first-pos2.first);
    unsigned dist = 11; if(pos1.second==0 || pos2.second==0) dist = 13;
    unsigned xdiff2 = dist-std::abs(pos1.first-pos2.first);
    return ydiff + std::min(xdiff1, xdiff2);
}

int main(int argc, char** argv)
{
    RiverCityRansom test;
    build_distancemap();

    if(argc > 1)
    {
        std::printf("Decoding of %s: %s\n", argv[1], test.Decode(argv[1]) ? "successful":"failure");
        std::printf("Stats=%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u Money=%X Inventory=%u-%u-%u-%u-%u-%u-%u-%u Mask=%02X-%02X\n",
            test.Stats[0],test.Stats[1],test.Stats[2],test.Stats[3],test.Stats[4],
            test.Stats[5],test.Stats[6],test.Stats[7],test.Stats[8],test.Stats[9], test.Stats[10],
            test.MoneyHex,
            test.Inventory[0],test.Inventory[1],test.Inventory[2],test.Inventory[3],
            test.Inventory[4],test.Inventory[5],test.Inventory[6],test.Inventory[7],
            test.Mask1, test.Mask2);
    }

    // Create a password
    test = RiverCityRansom{};
    for(auto& t: test.Stats) t = 63;
    test.Stats[8] = test.Stats[9] = 127;
    test.Stats[10] = 31;
    test.Money = 2000;
    test.Inventory[0] = 0x80;
    test.Inventory[1] = 0x7C;
    test.Inventory[2] = 0x7D;
    test.Inventory[3] = 0x7E;
    test.Inventory[4] = 0x7F;
    test.Inventory[5] = 0x48;
    test.Inventory[6] = 0x23;
    test.Inventory[7] = 0x1F;

    // 
    unsigned best[3] = {0,0,~0u};
    #pragma omp parallel for collapse(2) firstprivate(test)
    for(unsigned k=0; k<64; ++k)
    for(unsigned m=0; m<=99999; ++m)
    {
        test.Money = m;
        auto password = test.Encode(k);
        unsigned penalty = 0;
        for(unsigned n=1; n<password.size(); ++n) penalty += distance(password[n-1], password[n]);
        if(penalty < best[2] || best[2]==~0u)
        //if(penalty > best[2] || best[2]==~0u)
        {
            best[0]=m; best[1]=k; best[2]=penalty;
            //std::printf("Encoded as: %s with key %u, money %u\n", password.c_str(), k, m);
        }
    }
#if 1
    test.Money = best[0];
    unsigned key = best[1];
#else
    test.Money   = 2000;
    unsigned key = 0x7F;//'H'-'A';
#endif

    auto password = test.Encode(key);
    std::printf("Encoded as: %s with key: %u\n", password.c_str(), key);

    test = RiverCityRansom{};
    std::printf("Decoding of %s: %s\n", password.c_str(), test.Decode(password) ? "successful":"failure");
    std::printf("Stats=%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u Money=%X Inventory=%u-%u-%u-%u-%u-%u-%u-%u Mask=%02X-%02X\n",
        test.Stats[0],test.Stats[1],test.Stats[2],test.Stats[3],test.Stats[4],
        test.Stats[5],test.Stats[6],test.Stats[7],test.Stats[8],test.Stats[9], test.Stats[10],
        test.MoneyHex,
        test.Inventory[0],test.Inventory[1],test.Inventory[2],test.Inventory[3],
        test.Inventory[4],test.Inventory[5],test.Inventory[6],test.Inventory[7],
        test.Mask1, test.Mask2);
}
