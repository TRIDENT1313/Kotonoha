namespace kotonoha
{
    class textObject
    {
    public:
        kotonohaData::acessMapper *exportTo = NULL;
        void push(std::string text, std::string timeStart, std::string timeEnd, std::string type, std::string sceneName)
        {
            //Start ASS Header
            if (!exportTo->text.init)
            {
                exportTo->text.stream << "[Script Info]\nTitle:" << sceneName << "\nScriptType: v4.00+\nWrapStyle: 0\nScaledBorderAndShadow: yes\nYCbCr Matrix: None\n\n[V4+ Styles]\nFormat: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\nStyle: Default, Arial, 24, &H00FFFFFF, &H000000FF, &H00000000, &H80000000, -1, 0, 0, 0, 100, 100, 0, 0.00, 1, 2, 2, 2, 10, 10, 10, 1\n\n[Events]\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n";
                exportTo->text.init = true;
            }
            //Convert ([comand]=MM:SS:DD) and (MM:SS:DD;) to ASS format
            timeEnd.pop_back();
            std::vector<std::string> timeEndMMSSDD;
            std::vector<std::string> timeStartSplitComand;
            std::vector<std::string> timeStartMMSSDD;
            std::string token;
            std::istringstream stringStream(timeEnd);
            while (std::getline(stringStream, token, ':'))
            {
                timeEndMMSSDD.push_back(token);
            }
            stringStream.str("");
            stringStream = std::istringstream(timeStart);
            while (std::getline(stringStream, token, '='))
            {
                timeStartSplitComand.push_back(token);
            }
            stringStream.str("");
            stringStream = std::istringstream(timeStartSplitComand[1]);
            while (std::getline(stringStream, token, ':'))
            {
                timeStartMMSSDD.push_back(token);
            }
            //Push to ASS File, this line append one track sub to ass
            exportTo->text.stream << "Dialogue:0,0:" << timeStartMMSSDD[0] << ":" << timeStartMMSSDD[1] << "."  << timeStartMMSSDD[2] << ",0:" << timeEndMMSSDD[0] << ":" << timeEndMMSSDD[1] << "." << timeEndMMSSDD[2] << ",Default,,0,0,0,," << text << std::endl;
        };
    };
    inline std::pair<size_t, std::uint8_t> x2pos_mask(int x)
    {
        size_t r = x % CHAR_BIT;
        std::uint8_t mask = 0b10000000;
        mask >>= r;
        return {x, mask};
    }
    int playText(void *import)
    {
        kotonohaData::acessMapper *importedTo = static_cast<kotonohaData::acessMapper *>(import);
        int h, w;
        //Init ASS
        ASS_Library *ass_library;
        ASS_Renderer *ass_renderer;
        ass_library = ass_library_init();
        ass_renderer = ass_renderer_init(ass_library);
        ass_set_extract_fonts(ass_library, 1);
        ass_set_fonts(ass_renderer, NULL, "sans-serif", ASS_FONTPROVIDER_AUTODETECT, NULL, 1);
        ass_set_hinting(ass_renderer, ASS_HINTING_NATIVE);
        //Create new ASS Track
        ASS_Track *track = ass_new_track(ass_library);
        std::string subSs = importedTo->text.stream.str();
        char* str_c = new char[subSs.length() + 1];
        strcpy(str_c, subSs.c_str());
        ass_process_data(track,str_c,subSs.length() + 1);
        //Start
        kotonohaTime::delay(1000);
        importedTo->root->log0->appendLog("(Text) - Start");
        while (importedTo->control->outCode == -1)
        {
            
            if (importedTo->control->display[2])
            {
                //Get window size to update
                SDL_GetWindowSize(importedTo->root->window, &w, &h);
                ass_set_storage_size(ass_renderer, w, h);
                ass_set_frame_size(ass_renderer, w, h);
                ASS_Image *img = ass_render_frame(ass_renderer, track, kotonohaTime::convertToMs(importedTo->control->timer0.pushTime()), NULL);
                //This for loop send all tracks to display
                for (; img != NULL; img = img->next)
                {
                    if (img->w == 0 || img->h == 0)
                    {
                        continue;
                    }
                    //Convert ASS Img to RGBA
                    SDL_Texture *texture = SDL_CreateTexture(importedTo->root->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, img->w, img->h);
                    uint32_t *pixels = NULL;
                    int pitch = 0;
                    SDL_LockTexture(texture, NULL, reinterpret_cast<void **>(&pixels), &pitch);
                    unsigned char *bitmap = img->bitmap;
                    for (int y = 0; y < img->h; y++)
                    {
                        for (int x = 0; x < img->w; x++)
                        {
                            auto pos_mask = x2pos_mask(x);
                            auto filled = bitmap[pos_mask.first] & pos_mask.second;
                            pixels[x] = (filled) ? img->color : 0;
                        }
                        pixels = reinterpret_cast<std::uint32_t *>(reinterpret_cast<std::uintptr_t>(pixels) + pitch);
                        bitmap += img->stride;
                    }
                    //Send texture to display
                    SDL_UnlockTexture(texture);
                    SDL_Rect dst = {img->dst_x, img->dst_y, img->w, img->h};
                    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_MUL);
                    SDL_RenderCopy(importedTo->root->renderer, texture, NULL, &dst);
                    SDL_DestroyTexture(texture);
                }
                //End frame sub draw
                importedTo->control->display[2] = false;
                importedTo->control->display[3] = true;
            }
        };
        //End text engine
        ass_free_track(track);
        ass_renderer_done(ass_renderer);
        ass_library_done(ass_library);
        importedTo->root->log0->appendLog("(Text) - End");
        return 0;
    };
}