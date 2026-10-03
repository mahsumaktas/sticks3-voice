#include "voice_ui.h"
#include <cstdio>
#include <filesystem>
#include <string>

int main(int argc,char** argv) {
    const std::string dir=argc>1?argv[1]:"assets/previews";
    std::filesystem::create_directories(dir);
    struct Scenario {const char* name;voice_ui::State state;};
    const Scenario scenarios[]={{"ready",voice_ui::State::Ready},{"speaking",voice_ui::State::Speaking},
        {"waiting",voice_ui::State::WaitingForAudio},{"released",voice_ui::State::Released},
        {"usb",voice_ui::State::Disconnected},{"timeout",voice_ui::State::Timeout},{"error",voice_ui::State::Error}};
    for(bool codex:{false,true}) for(const auto& scenario:scenarios) {
        voice_ui::Renderer renderer;
        for(unsigned frame=0;frame<20;++frame) {
            voice_ui::View view{codex,scenario.state,frame*67,12000,static_cast<uint32_t>((frame*3727)%24000)};
            renderer.render(view);
        }
        const std::string path=dir+"/"+(codex?"codex-":"claude-")+scenario.name+".ppm";
        FILE* file=std::fopen(path.c_str(),"wb");if(!file)return 1;
        std::fprintf(file,"P6\n240 135\n255\n");
        for(int i=0;i<voice_ui::Width*voice_ui::Height;++i) {
            const uint16_t c=renderer.pixels()[i];
            const unsigned char rgb[]={static_cast<unsigned char>(((c>>11)<<3)|(c>>13)),
                static_cast<unsigned char>((((c>>5)&63)<<2)|((c>>9)&3)),
                static_cast<unsigned char>(((c&31)<<3)|((c&31)>>2))};
            std::fwrite(rgb,1,3,file);
        }
        std::fclose(file);
        std::printf("%s\n",path.c_str());
    }
}
