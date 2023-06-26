#include <Arduino.h>
#define SHORTEN 1
//String utfUpper = "TEST";
String UTFUppercase(String inputString, bool shorten = 0)  //converts UTF-8 CZ/SK/PL/DE/HU to uppercase
{
    int stringLength = sizeof(inputString);
    Serial.print(inputString);
    Serial.print(" > ");
    String outputString;
    for (int i=0; true; i++)
    {

        char c = inputString[i]; 
        if (c & 0x80)   //this is a 2-byte UTF-8 char!
        {
            
            i++;                          //we don't need ya now
            char d = inputString[i];
            if (c == 0xc3 && d == 0x9f) //ß is an exception
            {
                outputString += 'S'; outputString += 'S';
            } else { 
                outputString += c;
                if (c == 0xc3) d = d & 0xdf;  //3rd bitplane (-32)
                if ((c == 0xc4 && d < 0xb8) || (c == 0xc5 && d > 0x89 && d < 0xb8)) d = d & 0xfe;  //higher bitplane (-1 if odd)
                if ((c == 0xc4 && d > 0xb8) || (c == 0xc5 && (d < 0x89 || d > 0xb8))) d = (d | 0x01) - 2;  //anomaly: -1 if even
                outputString += d;
            }
        } else {        //this is an ASCII char
            if (c >= 0x61 && c <= 0x7a)     // ASCII a-z
                c = c & 0xdf;               // ASCII A-Z
            if (c) outputString += c;
        }
        if (!c) 
        {   
            //outputString += '\0';
            break;                           //end of string
        }
    }
    if(shorten)
        {
            outputString.replace(" HLAVNÍ", " HL.N.\xF0"); // \xF0 never occurs in CentrEU UTF-8, used to mark \0 (F0 = future NULL)
            outputString.replace(" HLAVNÁ", " HL.ST.\xF0");
            outputString.replace(" GL", " GL.\xF0");
            outputString.replace(" ZASTÁVKA", " ZAST.\xF0");
            outputString.replace(" (", "\xF0");
            outputString.replace(" MASARYK", " MAS.N.\xF0");
            outputString.replace("HRADEC ", "HR.");
            outputString.replace(" U ", " U\xF1");
            outputString.replace(" V ", " V\xF1");
            outputString.replace(" NA ", " \xFA\xF2.");
            outputString.replace(" POD ", " P.\xF2.");     // \xF2 never occurs in Central European UTF-8, used here to mark n.X.
            outputString.replace(" NAD ", " N.\xF2.");     // \xF2 never occurs in Central European UTF-8, used here to mark n.X.
            int a = outputString.indexOf('\xF2');         //replacing " nad Xyzerou" with " n.X.\0"
            if( a != -1 ) 
            {   
                outputString.setCharAt(a, outputString.charAt(a+2));
                if(outputString.charAt(a) & 0x80) // 2-byte UTF-8 at it again
                {
                    a++;
                    outputString.setCharAt(a, outputString.charAt(a+2));
                    outputString.setCharAt(a+1, '.');
                }
                int b, p;
                b = outputString.indexOf('-', a+2);
                p = outputString.indexOf(' ', a+2);
                if (b == -1 || p < b && p != -1) b = p; 
                if (b != -1)
                {
                    outputString.remove(a+2, b-a-1);
                } else outputString.remove(a+2, outputString.length()-a-2);
                outputString.replace("\xFA", "N.");
            }
            a = outputString.indexOf('\xF1');         //replacing " v Podkrkonoší" with " v1Podkrkonoší" and " v P.0"
            if( a != -1 ) 
            {   
                outputString.setCharAt(a, ' '); //clear flag
                if(outputString.charAt(a+1) & 0x80) a++;
                outputString.setCharAt(a+2, '.');
                int b, p;
                b = outputString.indexOf('-', a+2);
                p = outputString.indexOf(' ', a+2);
                if (b == -1 || p < b && p != -1) b = p; 
                if (b != -1)
                {
                    outputString.remove(a+3, b-a-2);
                } else outputString.remove(a+3, 99/*outputString.length()-a-2*/);
            }
            
            //outputString.replace('\xF0', '\0');  //WARNING! THIS DOES NOT ACTUALLY SHORTEN A String! FOLLOWING LINES ARE NEEDED
            a = outputString.indexOf("\xF0");
            if (a != -1) 
            { 
                outputString.remove(a, 99/*outputString.length()-a-2*/);
            }
        }

    Serial.println(outputString);
    return outputString;
}

String smallNums(const char* inputString) //change all numbers of input into small (7pt) ones for the Elektročas font
{
    char c = 1;
    String outputString;
    int i = 0;
    do 
    {
        c = inputString[i];
        if (c >= '0' && c <= '9') {outputString += char(0xc2); outputString += char(c + 0x60);}
        else outputString += char(c);
        i++;
    }
    while (c);
    return outputString;
}

String bigNums(const char* inputString) //change all numbers + colons of input into big (10pt) ones for the Elektročas font
{
    char c = 1;
    String outputString;
    int i, j = 0;
    do 
    {
        c = inputString[i];
        if (c >= '0' && c <= ':') {outputString += char(0xc2); outputString += char(c + 0x50);}
        else outputString += char(c);
        i++;
    }
    while (c);
    return outputString;
}

String UTFLowercase(const char* inputString)    //TODO, does not work yet: convert TRAINAMES to lowercase (Title Case \after UTF-8-2\RegioJet)
{
    int stringLength = sizeof(inputString);
    String outputString;
    uint8_t keepUppercase = 2;
    char c = 1;
    for (int i=0; c; i++)
    {
        c = inputString[i];
        if(keepUppercase == 1 && (c == ' ' || c == '-')) keepUppercase = 2;
        if(keepUppercase == 1 && (c != ' ' && c != '-')) keepUppercase = 0;
        if(keepUppercase < 2)
        { 
            if (c > 0xC0)   //this is a 2-byte UTF-8 char!
            {
                i++;                          //we don't need ya now
                char d = inputString[i];
                outputString += c;
                if (c == 0xc3) d = d | 0x20;  //3rd bitplane (+32)
                if ((c == 0xc4 && d < 0xb8) || (c == 0xc5 && d > 0x89 && d < 0xb8)) d = d | 0x01;  //higher bitplane (+1 if even)
                if ((c == 0xc4 && d > 0xb8) || (c == 0xc5 && (d < 0x89 || d > 0xb8))) d = (d + 1) & 0xfe;  //anomaly: +1 if odd
                if ((d == 0xad) && (i == 5) && (inputString[0] == 'J')) keepUppercase = 1; else keepUppercase = 0; //Jiří _
                outputString += d;
            } else {        //this is an ASCII char
                if (c >= 'A' && c <= 'Z')     // ASCII A-Z
                    c = c | 0x20;             // ASCII a-z
                outputString += c;
                if ((c == 'y') && (i == 2)) keepUppercase = 8; else keepUppercase = 1;
            }
        } else //keep uppercase, just print it
        {
            outputString += c;
            if(c < 0xC0 && c != ' ' && c != '-') keepUppercase -= 2; //this is not the first of a 2-byte character or space
        }
        
        if (!c) 
        {   
            //outputString += '\0';     //actually unneccessary
            break;                           //end of string
        }
    }
    return outputString;
}
String stringifyAbsTime(int minutes)
{
    String outputString = "";
    if(minutes/60 < 10 && tableTimeLeadingZero) outputString += '0';
    outputString += minutes/60;
    outputString += ':';
    if(minutes%60 < 10) outputString += '0';
    outputString += minutes%60;
    return outputString;
}

String fancifyTrainType(const char* inputString)
{
    String outputString = inputString; 
    outputString.replace("EC", "\xC2\x9C");
    outputString.replace("IC", "\xC2\x9D");
    outputString.replace("SC", "\xC2\x9E");
    return outputString;
}

int guessTrainSpeed(char* inputType)
{
    if(!strcmp(inputType, "EC")) return 2;
    if(!strcmp(inputType, "IC")) return 2;
    if(!strcmp(inputType, "SC")) return 2;
    if(!strcmp(inputType, "EN")) return 2;
    if(!strcmp(inputType, "rj")) return 2;
    if(!strcmp(inputType, "Ex")) return 2;
    if(!strcmp(inputType, "RJ")) return 2;
    if(!strcmp(inputType, "RJET")) return 2;
    if(!strcmp(inputType, "LET")) return 2;
    if(!strcmp(inputType, "LE")) return 2;
    
    if(!strcmp(inputType, "R")) return 1;
    if(!strcmp(inputType, "Rx")) return 1;

    return 0; //Os, Sp, BUS, ARR, TLX...
}

/*
void append(char* s, char c, int l)
{
    s[l] = c;
    s[l+1] = '\0';
}

void getUTFUpper(const char* inputChars, bool shorten = 0)  //converts UTF-8 CZ/SK/PL/DE/HU to uppercase
{
    int stringLength = sizeof(inputChars);
    append(utfUpper, 'X', 2);
    /*
    for (int i=0; 1; i++) //forever until break
    {
        if(strcmp(inputChars + i, " hlavní"))
            {utfUpper = strcat(utfUpper, " HL.N.\0"); break;} //hlavní nádraží → HL.N.
        if(strcmp(inputChars + i, " hlavná"))
            {utfUpper = strcat(utfUpper, " HL.ST.\0"); break;} //hlavná stanica → HL.ST.
        if(strcmp(inputChars + i, "Masary"))
            {utfUpper = strcat(utfUpper, " MAS.N.\0"); break;} //Masarykovo nádr. → MAS.N.
        if(strcmp(inputChars + i, " nad "))
            {utfUpper = strcat(utfUpper, " N.?.\0"); utfUpper[strlen(utfUpper)-2]=inputChars[i+5]; break;} //nad Xyz → N.X.
        if(strcmp(inputChars + i, "Hradec"))
            {utfUpper = strcat(utfUpper, "HR.\0"); i += 7;} //Hradec → HR.
        char c = inputChars[i]; 
        if (c & 0x80)   //this is a 2-byte UTF-8 char!
        {
            
            i++;                          //we don't need ya now
            char d = inputChars[i];
            if (c == 0xc3 && d == 0x9f) //ß is an exception
            {
                utfUpper = strcat(utfUpper, "SS");
                //utfUpper += 'S'; utfUpper += 'S';
            } else { 
                append(utfUpper, c, i);
                //utfUpper += c;
                if (c == 0xc3) d = d & 0xdf;  //3rd bitplane (-32)
                if ((c == 0xc4 && d < 0xb8) || (c == 0xc5 && d > 0x89 && d < 0xb8)) d = d & 0xfe;  //higher bitplane (-1 if odd)
                if ((c == 0xc4 && d > 0xb8) || (c == 0xc5 && (d < 0x89 || d > 0xb8))) d = (d | 0x01) - 2;  //anomaly: -1 if even
                append(utfUpper, d, i);
            }
        } else {        //this is an ASCII char
            if (c >= 0x61 && c <= 0x7a)     // ASCII a-z
                c = c & 0xdf;               // ASCII A-Z
            append(utfUpper, c, i);
        }
        if (!c) 
        {   
            append(utfUpper, '\0', i);
            //utfUpper += '\0';
            break;                           //end of string
        }
    }
    //return utfUpper;
    
}*/


/*const unsigned char utf8_latin2_LUT[256] = {
    193, 225, 196, 228, 161, 177, 198, 230, 200, 232,  67,  99, 200, 232, 207, 239,  
     68, 100,  69, 101, 204, 236,  69, 101, 202, 234, 204, 236,  71, 103,  71, 103,  
     71, 103,  71, 103,  72, 104,  72, 104,  73, 105,  73, 105,  73, 105,  73, 105,  
     73, 105,  74, 106,  74, 106,  75, 107, 107, 197, 229,  76, 108, 165, 181,  76, 
    108, 163, 179, 209, 241,  78, 110, 210, 242, 110,  78, 110,  79, 111, 214, 246, 
    213, 245, 214, 246, 192, 224,  82, 114, 216, 248, 166, 182,  83, 115, 170, 186, 
    169, 185,  84, 116, 171, 187,  84, 116, 219, 251,  85, 117,  85, 117, 217, 249, 
    219, 251,  85, 117,  87, 119,  89, 121,  89, 172, 188, 175, 191, 174, 190, 127,  
     98,  66,  66,  98,  66,  98, 127, 198, 230,  68, 169, 127, 127, 127, 174,  97,  
     69,  70, 102,  71, 127, 127, 127, 127,  75, 107, 185, 127, 127,  78, 190, 127, 
    160,  33, 127, 127, 127, 127, 124, 167, 127,  64, 127,  34, 127, 173,  64, 127, 
    176, 126, 127, 127, 127, 117, 127,  46, 127, 127, 127,  34, 127, 127, 127,  63, 
    193, 193,  65,  65, 196,  65,  65,  67, 201, 201,  69, 203,  73, 205, 206,  73,  
     68, 210, 211, 211, 212, 212, 214, 120, 127, 127, 218, 127, 220, 221, 127, 223, 
    225, 225,  97,  97, 228,  97,  97,  99, 233, 233, 101, 235, 237, 237, 238, 105, 
    127, 242, 243, 243, 244, 244, 246,  47, 111, 250, 250, 117, 252, 253, 127, 121,
};

unsigned char utf8_to_latin2(unsigned char left, unsigned char right)
{
    return utf8_latin2_LUT[((left & 0x03) << 6 ) | (right & 0x3f)];     //2 LSBs of left + 6 LSBs of right byte...
                //...is a sufficiently unique ID of a 2-byte UTF-8 CS/SK/DE/PL/HU character, used as LUT index
}*/ //We're NOT using Latin2