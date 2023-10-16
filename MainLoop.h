namespace kotonoha
{
    class loop
    {
    public:
        int game(SDL_Window *windowEntry, SDL_Renderer *rendererEntry, std::string path, kotonohaData::configsData fileConfigs, kotonoha::logger *log0, ImGuiIO *io)
        {
            // Create data structure
            kotonohaData::rootData *rootData = new kotonohaData::rootData;
            rootData->window = windowEntry;
            rootData->renderer = rendererEntry;
            rootData->log0 = log0;
            rootData->fileConfigs = &fileConfigs;
            rootData->io = io;
            kotonohaData::controlData *controlData = new kotonohaData::controlData;
            kotonohaData::acessMapper *global = new kotonohaData::acessMapper;
            global->control = controlData;
            global->root = rootData;
            rootData->image0 = new kotonohaData::imageData;
            rootData->audio0 = new kotonohaData::audioData;
            rootData->video0 = new kotonohaData::videoData;
            rootData->text0 = new kotonohaData::textData;
            rootData->question0 = new kotonohaData::questionData;
            // Export global to inside objects structs, this allow acess in all threads root var
            static_cast<kotonoha::audioObject *>(rootData->audio0)->exportTo = global;
            static_cast<kotonoha::videoObject *>(rootData->video0)->exportTo = global;
            static_cast<kotonoha::imageObject *>(rootData->image0)->exportTo = global;
            static_cast<kotonoha::textObject *>(rootData->text0)->exportTo = global;
            static_cast<kotonoha::questionObject *>(rootData->question0)->exportTo = global;
            // Loading script to create all game objects
            log0->appendLog("(ORS - Pre) - Reading " + path);
            controlData->outCode = kotonoha::orsLoader(global, path);
            // Start game loop and threads
            std::thread thread1(kotonoha::ui, global);
            std::thread thread2(kotonoha::playImage, global);
            std::thread thread3(kotonoha::playVideo, global);
            std::thread thread4(kotonoha::playAudio, global);
            std::thread thread5(kotonoha::playText, global);
            rootData->log0->appendLog("(ML) - Entry point to while");
            bool vsync = true;
            int vsyncStatus = vsync;
            SDL_RenderSetVSync(rendererEntry, vsyncStatus);
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
            while (controlData->outCode == 0)
            {
                // Event reciver
                while (SDL_PollEvent(&rootData->event))
                {
                    ImGui_ImplSDL2_ProcessEvent(&rootData->event);
                    controlData->outCode = keyBinds0(&rootData->event, windowEntry, 3, rendererEntry, &vsync, log0);
                }
                // Render send
                if (controlData->display[4])
                {
                    SDL_RenderPresent(rendererEntry);
                    controlData->display[0] = true;
                    controlData->display[4] = false;
                }
            }
            // Wait threads
            thread1.join();
            thread2.join();
            thread3.join();
            thread4.join();
            thread5.join();
            // Write on log the event that caused the close
            switch (controlData->outCode)
            {
            case 1:
                log0->appendLog("(ML) - Close");
                break;
            case 2:
                log0->appendLog("(ML) - Reset");
                break;
            case 3:
                log0->appendLog("(ML) - Return to menu");
                break;
            case 4:
                log0->appendLog("(ML) - Scene ended");
                break;
            case 5:
                log0->appendLog("(ML) - Erro on loading ORS file");
                break;
            default:
                log0->appendLog("(ML) - Uknonw return code");
                break;
            }
            int returnCode = controlData->outCode;
            rootData->log0->appendLog("(ML) - End");
            // Free objects
            delete static_cast<kotonoha::audioObject *>(rootData->audio0);
            delete static_cast<kotonoha::videoObject *>(rootData->video0);
            delete static_cast<kotonoha::imageObject *>(rootData->image0);
            delete static_cast<kotonoha::textObject *>(rootData->text0);
            delete global;
            delete controlData;
            delete rootData;
            return returnCode;
        };
    };
}