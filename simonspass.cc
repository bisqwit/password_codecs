#include <initializer_list>
struct SimonsQuestData
{
    unsigned char level{}, day{}, laurels{}, garlics{}, crystal{}, whip{};
    bool rib{},heart{},eye{},nail{},ring{}, silkbag{},rosary{},haslaurel{},hasgarlic{};
    bool dagger{},knife1{},knife2{},water{},diamond{},flame{},stake{};
public:
    bool Decode(const char* password)
    {
        unsigned char bytes[10], letters[16], daylow=0;
        // Convert symbols into letters
        for(unsigned c=0; c<16; ++c) letters[c] = SymbolToLetter(password[c]);
        // Decrypt
        unsigned xorindex = letters[15]&0xF, addvalue = (letters[15] >> 4) | ((letters[14] & 7) << 1);
        for(unsigned c=0; c<14; ++c) letters[c] = GetXor(letters[c]-addvalue, xorindex,c);
        // Convert letters into bytes
        ConvertBits<5,8>(letters, bytes);
        // Extract data
        ForAll(*this, [&](auto& item, auto pos, auto bits) { item = (bytes[pos/8] >> (pos%8)) & ((1 << bits)-1); },
                      [&](unsigned hi, unsigned lo)        { day = hi*10 + (daylow=lo); });
        // Verify data ranges and the checksum
        return bytes[0]<0x07 && bytes[1]<0x9A && bytes[2]<0x89 && bytes[3]<0x80 && bytes[4]<0x10
        &&     bytes[5]<0x80 && bytes[6]<0x05 && daylow  <10   && laurels<9
        && bytes[7]*256+bytes[8] == (bytes[0]+bytes[1]+bytes[2]+bytes[3]+bytes[4]+bytes[5]+bytes[6]);
    }
    void Encode(char* password, unsigned xorindex, unsigned addvalue) const
    {
        unsigned char bytes[10]{}, letters[16];
        // Insert data
        ForAll(*this, [&](const auto& item, auto pos, auto) { bytes[pos/8] |= unsigned(item) << (pos%8); });
        // Calculate and insert checksum, and insert the encryption specs
        bytes[7] = (bytes[0]+bytes[1]+bytes[2]+bytes[3]+bytes[4]+bytes[5]+bytes[6]) >> 8;
        bytes[8] = (bytes[0]+bytes[1]+bytes[2]+bytes[3]+bytes[4]+bytes[5]+bytes[6]) & 0xFF;
        bytes[9] = (addvalue << 4) | (xorindex);
        // Convert bytes into letters
        ConvertBits<8,5>(bytes, letters);
        // Encrypt
        for(unsigned c=0; c<14; ++c) letters[c] = GetXor(letters[c], xorindex,c) + addvalue;
        // Convert letters into symbols
        for(unsigned c=0; c<16; ++c) password[c] = LetterToSymbol(letters[c]);
    }
    void Encode(char* password, unsigned framecounter) const
    {
        Encode(password, framecounter&15, (032143210 >> (3*(framecounter&7))) & 7);
    }
private:
    template<unsigned inbits, unsigned outbits, unsigned insize>
    static void ConvertBits(const unsigned char (&in)[insize], unsigned char (&out)[insize*inbits/outbits])
    {
        for(unsigned cache=0, cachelen=0, inpos = 0, outpos = 0; inpos < insize; ++inpos)
        {
            cache = (cache << inbits) | (in[inpos] & ((1 << inbits)-1)); // Feed low bits
            for(cachelen += inbits; cachelen >= outbits; )
            {
                cachelen -= outbits;
                out[outpos++] = (cache >> cachelen) & ((1 << outbits)-1); // Eat high bits
                cache &= ((1 << cachelen) - 1);                           // Keep low bits
            }
        }
    }
    template<typename MaybeConstData, typename Functor, typename Functor2 = void(unsigned,unsigned)>
    static void ForAll(MaybeConstData& d, Functor&& f, Functor2 fixday = [](unsigned,unsigned){})
    {
        unsigned char dayhi = d.day/10, daylo = d.day%10, dummy = 0, pos = 0;
        auto j = [&](auto& item, unsigned bits=1) { f(item, pos, bits); pos += bits; };
        j(d.level,8);                    // Byte 0
        j(daylo,  4);   j(dayhi,4);      // Byte 1
        j(d.laurels,4); j(d.garlics,4);  // Byte 2
        j(d.rib);     j(d.heart);  j(d.eye); j(d.nail); j(d.ring); j(d.crystal,3); // Byte 3
        j(d.silkbag); j(d.rosary); j(d.haslaurel); j(d.hasgarlic); j(dummy,4);     // Byte 4
        j(d.dagger);  j(d.knife1); j(d.knife2); j(d.water); j(d.diamond); j(d.flame); j(d.stake); j(dummy); // Byte 5
        j(d.whip,8);                     // Byte 6
        fixday(dayhi,daylo);
    }
    static constexpr char GetXor(unsigned value, unsigned xorindex, unsigned index)
    {
        return value ^ SymbolToLetter("OBKQEPY1WH4SR3,EPY1WH4SLSCDR3,KBOQEMY1WHKSR3,ONK2EN01GH4SV3,"
        "ORKQEDY1WHSSR3,TJKSEDY1SHWKQV,CNKSEPK1SHYSR3,A3KYELY0UH4UR3,ONOSMP01WH4SV3,OBKQEPY1UHASR3,"
        "ABCDEFGHIJKLR3,QNKWEPY1WH4S33,CDKSEPYLWHGWB3,YFK0ENY1WD0WXV,DFKCENIP4LSWRE,4FK0HN05XA0WWV"[xorindex*15+index]);
    }
    static constexpr char SymbolToLetter(char c) { return (c<='9' ? (c-'0'+26) : (c-'A')) & 0x1F; }
    static constexpr char LetterToSymbol(char c) { return (c<26 || c>=36) ? ((c&31)+'A') : (c-26+'0'); }
};
