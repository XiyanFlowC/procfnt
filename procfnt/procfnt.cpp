// procfnt.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <cstdio>
#include <iostream>

// 请添加包含路径，以包含 zlib.h
extern "C" {
#include <zlib.h>
}

#include "liteopt.h"
#include "FontFile.h"
#include "Bitmap.h"

// 改变此行，使适应你的 zlib 路径
#pragma comment(lib, "D:\\Library\\zlib-1.2.12\\contrib\\vstudio\\vc14\\x64\\ZlibStatReleaseWithoutAsm\\zlibstat.lib")
/*
* cfg_type (int32) layout:
* +7=*7=*7------4+3+2---0*7-6+5+4+3-2+1-0+
* |~~~~~   r     |a|    g    |X|x| m | t |
* +==*==*--------+-+-----*---+-+-+---+---+
* x: save the flag whether cut the texture size from the block size when pack (if width less than 8, cut down
* m: save the mode
*   0 - nothing special
*   1 - save textures seperately (in the output folder
*   2 - unused
*   3 - unused
* t: save the output type
*   0 - output nothing (for test)
*   1 - save as bmp
*   2 - (reserved for) save as png (not available)
*   3 - unused
* X: save if extend the bmp rgbquad (colour index) usage, if it is set, the reserved field will be filled with alpha
* g: the font group [0,35] to be manuplated.
* a: proc all group, ignore g.
* r: reserved
* 
* cfg_block_h cfg_block_w
* when unpack/repack under the condition that cfg_type.m == 0, these configuration used to decade the texture
* size taken in the big image.
* 
* cfg_pcidx
* palette transparent colour index (for size cutting)
*/
int cfg_mode = 0, cfg_type = 0, cfg_bitcount = 16, cfg_block_h = 16, cfg_block_w = 16, cfg_full_alpha = 128;
int cfg_outfile_h = cfg_block_h * 64, cfg_outfile_w = cfg_block_w * 64, cfg_pcidx = 0;
const char* cfg_infn = nullptr, * cfg_outfn = nullptr, * cfg_subpalette = nullptr, * cfg_oripalette = nullptr, * cfg_alternm = nullptr;

#ifdef WIN32
#define strdup _strdup
#endif

void BlandPalette(Palette& pl, const Pixel& bgc)
{
#define ALPHA_CALC_PF(t, bg, val) ((t).val = (t).val * (t).alpha / cfg_full_alpha + (bg).val * (cfg_full_alpha - (t).alpha) / cfg_full_alpha)
    int size = pl.Size();
    for (int i = 0; i < size; ++i)
    {
        Pixel p = pl.GetPixel(i);
        ALPHA_CALC_PF(p, bgc, red);
        ALPHA_CALC_PF(p, bgc, blue);
        ALPHA_CALC_PF(p, bgc, green);
        p.alpha = bgc.alpha;
        pl.SetPixel(i, p);
    }
#undef ALPHA_CALC_PF
}

void ExtractGroup(FontFile& font, int grpidx, Palette pl);
void ImportGroup(FontFile& font, int grpidx);
int GraphicSizeReduce(Graphic** gr, const Palette& p);

int main(int argc, const char** argv)
{
    std::cout << argv[0] << std::endl;

    lopt_regopt("repack", 'r', 0, [](const char* param)->int
        {
            cfg_mode = 1;
            return 0;
        });
    lopt_regopt("unpack", 'u', 0, [](const char* param)->int
        {
            cfg_mode = 2;
            return 0;
        });
    lopt_regopt("input", 'i', 0, [](const char* param)->int
        {
            if (param == nullptr) return -1;
            cfg_infn = strdup(param);
            return 0;
        });
    lopt_regopt("output", 'o', 0, [](const char* param)->int
        {
            if (param == nullptr) return -1;
            cfg_outfn = strdup(param);
            return 0;
        });
    /*lopt_regopt("method", 'm', 0, [](const char* param)->int
        {
            return -1;
        });*/
    lopt_regopt("bmp", 'b', 0, [](const char* param)->int
        {
            cfg_type = (cfg_type & ~0x3) | 0x1;
            return 0;
        });
    lopt_regopt("split", 's', 0, [](const char* param)->int
        {
            cfg_type = (cfg_type & ~0xc) | 0x4;
            return 0;
        });
    lopt_regopt("template", 't', 0, [](const char* param)->int
        {
            cfg_alternm = strdup(param);
            return 0;
        });
    lopt_regopt("block-height", 'h', 0, [](const char* param)->int
        {
            if (param == nullptr) return -1;
            if(sscanf(param, "%8d", &cfg_block_h) != 1) return -1;
            return 0;
        });
    lopt_regopt("canvas-height", 'H', 0, [](const char* param)->int
        {
            if (param == nullptr) return -1;
            if (sscanf(param, "%8d", &cfg_outfile_h) != 1) return -1;
            return 0;
        });
    lopt_regopt("block-width", 'w', 0, [](const char* param)->int
        {
            if (param == nullptr) return -1;
            if(sscanf(param, "%8d", &cfg_block_w) != -1) return -1;
            return 0;
        });
    lopt_regopt("canvas-width", 'W', 0, [](const char* param)->int
        {
            if (param == nullptr) return -1;
            if (sscanf(param, "%8d", &cfg_outfile_w) != 1) return -1;
            return 0;
        });
    lopt_regopt("minimize-texture", 'x', 0, [](const char* param)->int
        {
            cfg_type = (cfg_type & ~0x10) | 0x10;
            return 0;
        });
    lopt_regopt("bit-count", 'B', 0, [](const char* param)->int
        {
            if (1 != sscanf(param, "%4d", &cfg_bitcount))
            {
                std::cout << "-B: invalid parameter :" << param;
            }
            return 0;
        });
    lopt_regopt("original-palette", 'p', 0, [](const char* param)->int
        {
            if (param == NULL) return -1;
            cfg_oripalette = strdup(param);
            return 0;
        });
    lopt_regopt("substitude-palette", 'P', 0, [](const char* param)->int
        {
            if (param == NULL) return -1;
            cfg_subpalette = strdup(param);
            return 0;
        });
    lopt_regopt("full-alpha", 'A', 0, [](const char* param)->int
        {
            if (1 != sscanf(param, "%4d", &cfg_full_alpha)) return -1;
            return 0;
        });
    lopt_regopt("proc-grp", 'g', 0, [](const char* param)->int
        {
            int val;
            if (1 != sscanf(param, "%4d", &val)) return -1;

            cfg_type = (cfg_type & ~0x7c0) | ((val & 0x1f) << 6);

            return 0;
        });
    lopt_regopt("proc-all", 'a', 0, [](const char* param)->int
        {
            cfg_type = (cfg_type & ~0x800) | 0x800;
            return 0;
        });
    lopt_regopt("bmp-rgba-quad", 'X', 0, [](const char* param)->int
        {
            cfg_type = (cfg_type & ~0x20) | 0x20;
            return 0;
        });
    lopt_regopt("cut-size", 'c', 0, [](const char* param)->int
        {
            cfg_type |= 0x8;
            return 0;
        });
    lopt_regopt("help", '?', 0, [](const char* param)->int
        {
            std::cout << "PES/WE 2014 Font File Batch Processor by xiyan" << std::endl;
            std::cout << "ZLib Version: " << zlibVersion() << std::endl;
            std::cout << "This edition compiled for the header holding " << FONT_TEX_GRP_NUMBER << " groups." << std::endl;
            std::cout << "--input              -i [file name] specify input file." << std::endl;
            std::cout << "--output             -o [file name] specify output file." << std::endl;
            std::cout << "--template           -t [file name] specify template file. (valid only when repack)" << std::endl;
            std::cout << "--repack             -r pack the font." << std::endl;
            std::cout << "--unpack             -u unpack the font." << std::endl;
            std::cout << "--bmp                -b specify the output format as windows bmp file." << std::endl;
            std::cout << "--proc-grp           -g [group id] process the specified group." << std::endl;
            std::cout << "--proc-all           -a process all groups." << std::endl;
            std::cout << "--split              -s output seperately." << std::endl;
            std::cout << "--original-palette   -p [palette name] set the font file's palette." << std::endl;
            std::cout << "--substitude-palette -P [palette name] set the bmp file's palette." << std::endl;
            std::cout << "--block-width        -w [block width] specify the size of a single block in output picture." << std::endl;
            std::cout << "--block-height       -h [block height] specify the size of a single block in output picture." << std::endl;
            std::cout << "--canvas-width       -W [output file width] specify the size of output file." << std::endl;
            std::cout << "--canvas-height      -H [output file height] specify the size of output file." << std::endl;
            std::cout << "--cut-size           -c Minimise the width of the output (pack only)." << std::endl;
            //std::cout << "--cut-colour         -C [palette index] specify the colour index should be cutted." << std::endl;
            //std::cout << "--method  -m    specify the work method, see the document." << std::endl;
            std::cout << "--help               -? show this message." << std::endl;
            std::cout << "\nExample\n" <<
                "\tprocfnt -abu -i fnt.bin -o ./font_ex/  # to all groups, output as bmp, unpack the file; input is fnt.bin and output directory is ./font_ex/" << std::endl;
            std::cout << "\tprocfnt -bu -g 1 -i fnt.bin -o ./  # output as bmp, unpack the file; process group 1 only; input is fnt.bin and output to ./" << std::endl;
            std::cout << "\tprocfnt -br -g 1 -t fnt.bin -o out.bin -i ./ # the source format is bmp, repack the file, only modify the group 1, using fnt.bin as template, output to out.bin, using contents in ./ to repack." << std::endl;
            std::cout << "\tprocfnt -bra -i ./ -o out.bin # the source format is bmp, repack the file, rebuild all groups, using contents in ./ as input, output to file out.bin" << std::endl;
            return 0;
        });
    lopt_parse(argc, argv);
    lopt_finalize();

    try {
        // opt check & err / warn
        if (cfg_infn == nullptr || cfg_outfn == nullptr)
        {
            std::cerr << "Input and/or output filename have not set yet, using -i and/or -o to specify input/output filename(s).";
            exit(-1);
        }

        if (!((cfg_type >> 5) & 1) && cfg_oripalette == nullptr && cfg_mode == 2)
        {
            std::cerr << "Warning! blend enabled but not save blended original palette, may caused data lost." << std::endl;
            std::cerr << "Note: the blending is a feature to avoid the alpha chanel be output to bmp since bmp doesn't support it. however, it will change the palette of the output." << std::endl;
            std::cerr << "Note: so, you can using both -p to specify a palette file to store modiffied font palette, or -X to store original palette to bmp file." << std::endl;
            std::cerr << "Note: and by this, after you modify the texture, you can using -P to specified the outputed palette by -p in unpacking to restore the original palette mapping." << std::endl;
        }

        if (cfg_mode == 0)
        {
            std::cerr << "Mode not set. Using -p or -u to specify a mode.\n" << std::endl;
            exit(-1);
        }

        if (cfg_mode == 1) // pack
        {
            FontFile font;

            if (cfg_alternm != nullptr) // load template
            {
                font.SetFilePath(cfg_alternm);
                font.LoadFile();
            }
            // else font.SubPalette(Palette(cfg_oripalette));
            
            font.SetFilePath(cfg_outfn);

            if ((cfg_type >> 11) & 1) // flag a
            {
                for (int i = 0; i < FONT_TEX_GRP_NUMBER; ++i)
                {
                    ImportGroup(font, i);
                }
            }
            else
            {
                ImportGroup(font, (cfg_type >> 6) & 0x1F);
            }

            Palette* plt = nullptr;
            if (cfg_oripalette != nullptr)
            {
                plt = new Palette(cfg_oripalette);
                font.SubPalette(*plt);
            }
            else if (cfg_alternm == nullptr)
            {
                // UNDONE: Implemet the palette extraction from textures
                std::cerr << "Cannot generate a palette, using -p to specify a palette to use." << std::endl;
                exit(-15);
            }

            font.SaveFile();
            if (plt != nullptr) delete plt;
        }
        else // unpack
        {
            FontFile font(cfg_infn);

            if (!((cfg_type >> 5) & 1)) // not rbga palette
            {
                BlandPalette(font.GetPalette(), Pixel(0, 0, 0, 0)); // bland with black to get rid of alpha ch.
            }

            if (cfg_oripalette != nullptr) // to save original (font file) palette
            {
                auto& pl = font.GetPalette();

                pl.SetFilePath(cfg_oripalette);
                pl.SaveFile();
            }

            Palette pl;
            if (cfg_subpalette != nullptr) // to substitude original palette to save other colors
            {
                pl.SetFilePath(cfg_subpalette);
                pl.LoadFile();
            }
            else
            {
                pl = font.GetPalette();
            }

            if ((cfg_type >> 11) & 1) // flag a
            {
                for (int i = 0; i < FONT_TEX_GRP_NUMBER; ++i)
                {
                    ExtractGroup(font, i, pl);
                }
            }
            else
            {
                ExtractGroup(font, (cfg_type >> 6) & 0x1F, pl);
            }
        }
    }
    catch (exception& ex)
    {
        std::cerr << "Top Level Exception Catcher: Exec failed\n===============\n" << ex.ToString() << std::endl;
        exit(-15);
    }
    catch (bad_format& ex)
    {
        std::cerr << "Format Error\n==============\n" << ex.ToString() << std::endl;
        exit(-9);
    }
    catch (bad_operation& bo)
    {
        std::cerr << "Operation Error\n==============\n" << bo.ToString() << std::endl;
        exit(-10);
    }
    catch (std::exception& ex)
    {
        std::cerr << "std err, " << ex.what() << std::endl;
        exit(-16);
    }
    return 0;
}

int GetNextCodePoint(FILE* f) // not real code point of unicode, just a seq of UTF-8
{
    int ch0 = fgetc(f);
    if (ch0 == EOF) return 0;

    int ret = ch0;
    int sz = -1;
    while (ch0 & 0x80)
    {
        ch0 <<= 1;
        sz++;
    }
    if (sz == -1) return ret;

    while (sz--)
    {
        ret <<= 8;
        ret |= fgetc(f);
        //ch0 = fgetc(f);
        //if (ch0 == EOF) throw bad_format("table file", ftell(f), std::string("Unexpected ending (last ch ") + std::to_string(sz));
    }
    return ret;
}

void ImportGroup(FontFile& font, int grpidx)
{
    FILE* code_list = fopen((std::string(cfg_infn) + "/grp_" + std::to_string(grpidx) + ".txt").c_str(),
        "rb");
    if (code_list == NULL)
    {
        std::cout << "cannot open definition file, skipping group " << grpidx << std::endl;
        return;
    }

    std::cout << "importing group " << grpidx << std::endl;
    std::vector<FontTexture*>* grp = new std::vector<FontTexture*>();
    if ((cfg_type >> 2) & 1) // load seperately
    {
        std::string infolder = std::string(cfg_infn) + "/grp_" + std::to_string(grpidx) + "/";

        int i = 0;
        int codepoint = 0;
        while (codepoint = GetNextCodePoint(code_list))
        {
            Bitmap bm(infolder + std::to_string(i) + ".bmp");

            FontTexture* tex;
            if (cfg_subpalette == nullptr)
                tex = new FontTexture(bm, font.GetPalette());
            else
                tex = new FontTexture(bm, Palette(cfg_subpalette));

            tex->SetCodePoint(codepoint);
            grp->push_back(tex);

            ++i;
        }
    }
    else
    {
        Bitmap* bm = new Bitmap(std::string(cfg_infn) + "/grp_" + std::to_string(grpidx) + ".bmp");

        int codepoint;
        int line = 0, row = 0;

        cfg_outfile_h = bm->Height();
        cfg_outfile_w = bm->Width();

        Palette pl;
        if (nullptr != cfg_subpalette)
        {
            pl.SetFilePath(cfg_subpalette);
            pl.LoadFile();
        }
        else
        {
            pl = font.GetPalette();
        }

        while (codepoint = GetNextCodePoint(code_list))
        {
            Graphic* gr = bm->Extract(line, row, cfg_block_w, cfg_block_h); // TODO: add config file to determaine the w and h to control the gr size.
            int w = 0;
            if (cfg_type & 0x8)
                w = GraphicSizeReduce(&gr, pl);
            row += cfg_block_w;
            if (row >= cfg_outfile_w) row = 0, line += cfg_block_h;

            FontTexture* tex = new FontTexture(*gr, pl);
            tex->SetCodePoint(codepoint);
            
            // post proc
            if (codepoint == ' ') tex->SetWidth(0xC);
            else if (w) tex->SetWidth(w);
            else tex->SetWidth(0xF);

            grp->push_back(tex);
            // std::cout << tex->GetConsoleDemo() << std::endl;

            delete gr;
        }
        delete bm;
    }
    // grp built up
    font.GimmGroup(grpidx, grp);

    fclose(code_list);
}

void ExtractGroup(FontFile& font, int grpidx, Palette pl)
{
    std::cout << "extracting group " << grpidx << std::endl;
    auto grp = font.GetTextureGroup(grpidx);
    if (grp.size() == 0) return;

    FILE* code_list = 
        fopen((std::string(cfg_outfn) + "/grp_" + std::to_string(grpidx) + ".txt").c_str(),
            "wb");

    //if (!((cfg_type >> 5) & 1)) // not rbga palette
    //{
    //    BlandPalette(pl, Pixel(0, 0, 0, 0)); // bland with black to get rid of alpha ch.
    //}

    Graphic* g = nullptr;
    switch (cfg_type & 0x3)
    {
    case 0:
        for (int i = 0; i < grp.size(); ++i)
        {
            if (grp[i] == nullptr) continue;
            std::cout << "texture " << i << "(" << std::to_string(grp[i]->GetInfo().codepoint) << ")" << std::to_string(grp[i]->GetInfo().canv_w) << "*" << std::to_string(grp[i]->GetInfo().canv_h) << std::endl << grp[i]->GetConsoleDemo();
        }
        fclose(code_list);
        return;
        break;
    case 1:
        if ((cfg_type >> 2) & 1) // save seperately
        {
            std::cout << "hint: to make -s work correctly, you should create sub-folders named as 'grp_0', 'grp_1', ... under the [" << cfg_outfn << "] folder.";
            std::string outfolder = std::string(cfg_outfn) + "/grp_" + std::to_string(grpidx) + "/";

            for (int i = 0; i < grp.size(); ++i)
            {
                if (grp[i] == nullptr) continue;
                Bitmap out(0, 0);
                out.SetFilePath(outfolder + std::to_string(i) + ".bmp");
                grp[i]->GetTexture(out, pl);
                word cpr = grp[i]->GetInfo().codepoint;
                word cp = 0;
                while (cpr)
                {
                    cp = (cp << 8) | (cpr & 0xff);
                    cpr >>= 8;
                }
                while (cp)
                {
                    fputc((char)(cp & 0xff), code_list);
                    cp >>= 8;
                }
                out.SaveFile();
            }
            fclose(code_list);
            return;
        }
        g = new Bitmap(cfg_outfile_w, cfg_outfile_h);
        break;
    case 2:
    case 3:
        std::cerr << "not implemented yet." << std::endl;
        return;
    }
    if (g == nullptr) return;
    g->SetFilePath(std::string(cfg_outfn) + "/grp_" + std::to_string(grpidx) + ".bmp");

    int u = 0, v = 0;
    for (int i = 0; i < grp.size(); ++i)
    {
        if (grp[i] == nullptr) continue;
        Bitmap* part = new Bitmap();
        grp[i]->GetTexture(*part, pl);
        //std::cout << grp[i]->GetConsoleDemo() << std::endl;
        g->Paste(u, v, *part);
        v += cfg_block_w;
        if (v >= cfg_outfile_w) v = 0, u += cfg_block_h;
        if (u >= cfg_outfile_h)
        {
            std::cerr << "outfile size too small! using -H and -W to specify a larger number!" << std::endl;
            std::cerr << "current config: " << cfg_outfile_w << "*" << cfg_outfile_h << std::endl;
            exit(-1);
        }
        word cpr = grp[i]->GetInfo().codepoint;
        word cp = 0;
        while (cpr)
        {
            cp = (cp << 8) | (cpr & 0xff);
            cpr >>= 8;
        }
        while (cp)
        {
            fputc((char)(cp & 0xff), code_list);
            cp >>= 8;
        }
        delete part;
    }
    g->SaveFile();
    delete g;
    fclose(code_list);
}

#ifndef MIN(x, y)
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

int GraphicSizeReduce(Graphic** p_gr, const Palette& p) {
    Graphic* gr = *p_gr;
    int lm = 32, rm = -1, f = 0;

    // find left & right edge
    for (int i = 0; i < gr->Height(); ++i) {
        for (int j = 0; j < gr->Width(); ++j) {
            if (f) {
                if (p.GetPaletteIndex(gr->GetPixel(i, j)) != cfg_pcidx) {
                    rm = MAX(rm, j + 2);
                }
            }
            else {
                if (p.GetPaletteIndex(gr->GetPixel(i, j)) != cfg_pcidx) {
                    f = 1;
                    lm = MIN(lm, j - 1);
                    rm = MAX(rm, j + 2);
                }
            }
        }
        f = 0;
    }
    if (rm == -1) return 0; // empty graphic

    if (lm < 0) lm = 0;
    if (rm > gr->Width()) rm = gr->Width(); // clamp

    Graphic* ng = gr->Extract(lm, 0, (rm - lm + 7) & ~7, gr->Height()); // align to 8
    *p_gr = ng;
    delete gr;
    return rm - lm;
}
