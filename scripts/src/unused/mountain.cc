#include "../../lib/bin/easy.h" // gmeng import

extern "C" int gmeng_script_command( Gmeng::EventLoop* ev ) {
    Gmeng::Renderer::Model tree_model = create_model(24, 14);
    GAME_LOG("writing mountain model to chunk 1");
    tree_model.attach_texture(Gmeng::LoadTexture("envs/models/mountain.gt"));
    tree_model.position = { 16, 20 };
    ev->level->chunks.at(0).models.push_back(
        tree_model
    );
    return 2;
};
