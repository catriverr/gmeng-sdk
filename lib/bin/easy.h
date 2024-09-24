#include "cli/index.h"
#include "gmeng.h"

#include "src/textures.cpp"
#include "src/renderer.cpp"

/// GMENG EASY ANNOTATIONS
/// FOR EASIER USAGE OF THE ENGINE
/// LESS CAPABLE WITH OPTIONS BUT WILL BE MORE STABLE
/// AND BETTER FOR NEWER USERS


/// @since 9.1.0

/// GLOBALS

#define gmeng_version Gmeng::version
#define gmeng_gobal Gmeng::global
#define gmeng_functree Gmeng::functree

#define set_gmeng_arguments patch_argv_global
#define set_gmeng_cli_arguments Gmeng_Commandline::patch_argv

/// VGM Methods
#define vgm Gmeng::vgm_defaults

#define vgm_textures Gmeng::vgm_defaults::vg_textures
#define vgm_rmodels Gmeng::vgm_defaults::vg_rmodels

#define vgm_load Gmeng::_uread_into_vgm

#define vgm_texture(x) Gmeng::vd_find_texture( vgm_textures, x )
#define vgm_model(x) Gmeng::vd_find_model( vgm_rmodels, x )

/// THREADS

#define thread_catchup _gthread_catchup
#define join_threads Gmeng::_ujoin_threads
#define clear_threads Gmeng::_uclear_threads

#define LOG gm_log

/// RENDERER

#define DELTA_X Gmeng::_vcreate_vp2d_deltax
#define DELTA_Y Gmeng::_vcreate_vp2d_deltay

#define dp Gmeng::Renderer::drawpoint
#define vp Gmeng::Renderer::viewpoint

#define create_texture Gmeng::Renderer::generate_empty_texture
#define create_model Gmeng::Renderer::generate_empty_model

#define get_viewpoint Gmeng::Renderer::conv_dp

#define level_base_image Gmeng::_vget_renderscale2dpartial_scalar
#define level_current_image Gmeng::get_lvl_view
#define level_set_image Gmeng::emplace_lvl_camera
