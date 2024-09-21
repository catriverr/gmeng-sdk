<div align="center">
  <img src="https://allahcorp.com/static/assets/gmeng-more-hd.png##">
  <h1>Contributing to the Gmeng SDK</h1>
</div>

> As any other free and open-source software, Gmeng greatly values contributions. Any information & guide you may require to set up a development environment to contribute
> to developing the Gmeng Game Engine can be found in this document.

## Setting Up a Development Environment
> To get started, clone the repository & its submodules with:
> ```sh
>git clone https://github.com/catriverr/gmeng-sdk
>git submodule update --init --recursive
> ```
> This will set up all utilities & dependencies required by Gmeng.

## How to Contribute
Contributions must follow these guidelines in order to be better understandable and scalable.
> [!IMPORTANT]
> If your contribution is related to a bug/bug report, please visit [`gmeng.org/report`](https://gmeng.org/report) first.

### Commit Guidelines
> Your commits do not need to be split into reasonably-sized chunks. As its nature, Gmeng is a big project.
> However, your commits must be fixed on one single purpose. For example, this type of commit will (most probably) not be accepted:
> ```diff
> @@ 630, 20 @@ renderer.cpp
> inline Renderer::drawpoint _vget_vp2d_deltax(Renderer::viewpoint vp) {
> +return vp.end.x - vp.start.x;
> }
> @@ 1158, 20 @@ renderer.cpp
> inline void emplace_lvl_camera(Gmeng::Level, Gmeng::VectorView) {
> +ASSERT("pref.log", jWRAP::condition( DISABLE_IF( IS_SET Gmeng::global.shush ) ));
> @@ 1178, 20 @@ renderer.cpp
> +Gmeng::GameWindow sdl_screen = Gmeng::Init_SDL();
> ```
> This code makes it harder to track and trace bugs/issues in the software,
> or what the commit is about as it's not specific.
>
> Use comments in code where neccessary. Arbitary magic numbers, strings, byte shifts or other manipulation of execution
> makes the source code abstract unless it is supported with comments. This makes it easier for scaling existent code.

> [!NOTE]
> We will not accept commits for deprecated utilities unless it is for backwards-compatibility, as it is generally not a good idea.
> When contributing, focus on supported functionality. Please visit [`gmeng.org/goals`](https://gmeng.org/goals) and [`gmeng.org/subsystems`](https://gmeng.org/subsystems)
> for welcomed commits; currently supported functionality, namespaces and utilities that you are encouraged to contribute to.

### Naming Commits
> For improvement-based commits, use the `chore` naming principle:
> ```
>chore(networking): update server_fd->accept() to log the remote's IP Address
> ```
> For minor variable changes/tunes to code, use the `tune` naming principle:
> ```
> tuned frame_delay_ms to 12 (from 20) 
> ```
> For big changes, explain all functionality changes in your commit:
> ```
>8.2.3(pull#49): removed __FILE__, __LINE__ calls from gm_log, functree now uses __PRETTY_FUNCTION__
> ```

## Contributing to the Documentation
> For information about how to contribute to documenting the Gmeng Engine, visit the [`catriverr/gmeng-org`](https://github.com/catriverr/gmeng-org) repository.
