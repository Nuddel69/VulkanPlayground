SOURCE="shader.slang"

CC="$HOME/Projects/Vulkan/vulkansdk/default/x86_64/bin/slangc"
ARCH="spirv"
PROFILE="spirv_1_4"
VENTRYPOINT="vertMain"
FENTRYPOINT="fragMain"
OUTPUT="slang.spv"
FLAGS="-target $ARCH -profile $PROFILE -emit-spirv-directly -fvk-use-entrypoint-name -entry $VENTRYPOINT -entry $FENTRYPOINT"

$CC $SOURCE $FLAGS -o $OUTPUT
