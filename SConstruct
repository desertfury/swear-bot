env = Environment()

env.Append(CXXFLAGS=["-std=gnu++0x", "-O3", "-g", "-isystem", "/usr/include/pd/fixinclude", "-fPIC"])
env.Append(LINKFLAGS=["-std=gnu++0x", "-g"])
env.Append(LINKPATH=["."])
env.Append(CPPPATH=["."])
env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME']=1

swear_bot = env.Library("swear_bot", Glob("*.C"))

mod_bsswear_bot = env.SharedLibrary(
    "mod_swear_bot.so",
    Glob("*.C"),
    LIBS=["pd-base.s", "pd-pi.s"],
    SHLIBPREFIX="",
    LIBPATH="."
)
