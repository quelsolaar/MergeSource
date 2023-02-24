#!/bin/bash

function checkBuildMode()
{
    local buildMode=$1
    
    if [ "$buildMode" == "debug" ]; then
        echo "debug"
    else
        echo "release"
    fi
}

iosSysRoot="$( xcrun --sdk iphonesimulator --show-sdk-path )"
scriptPath="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

errorValue=1
successValue=0

justPrintHelp=0
skipVerification=0
buildForDevice=0

buildMode=""

projectName="zenith"
executableName="zenith_ios"
bundleId="com.shimmer.zenith"

iconName="shimmer"
version="9"
subVersion="0"

resourcePath="$scriptPath/res"
assetsPaths=("$scriptPath/../default" "$scriptPath/../theme_southbank" )

sourceFileDirectory="$scriptPath/../"

betraySourceFiles=("b_main.c" "b_ios_main.c" "b_ios_key_codes.c")
forgeSourceFiles=("f_math.c" "f_matrix_make.c" "f_matrix_operations.c" "f_mem_debug.c" "f_noise.c" "f_path.c" "f_sort.c" "f_text.c" "f_types.c")
imagineSourceFiles=("i_dir.c" "i_library.c" "i_settings.c" "i_threads.c" "i_time.c")
relinquishSourceFiles=("r_draw_buffer.c" "r_draw_extensions.c" "r_draw_framebuffer.c" "r_draw_parser.c" "r_draw_primitive.c" "r_draw_shader.c" "r_draw_state.c" "r_draw_uniform.c" "r_matrix.c" )
seduceSourceFiles=("s_background.c" "s_background_old.c" "s_draw_3d.c" "s_font_abel.c" "s_font_arial.c" "s_font_eurasia.c" "s_font_impact.c" "s_font_lane.c" "s_font_lato.c" "s_font_luke.c" "s_font_oswald.c" "s_font_sansumi.c" "s_font_times.c" "s_font_verdana.c" "s_lens.c" "s_lines.c" "s_main.c" "s_manipulator_point.c" "s_manipulator_rotate.c" "s_manipulator_scale.c" "s_manipulator_space.c" "s_particles.c" "s_popup.c" "s_popup_detect.c" "s_projection.c" "s_select.c" "s_settings_window.c" "s_settings_window2.c" "s_sort.c" "s_text.c" "s_text_edit.c" "s_text_exporter.c" "s_text_file_format.c" "s_text_output.c" "s_text_widget.c" "s_translate.c" "s_widget.c" "s_widget_radial.c" "s_widget_visualizers.c")
testifySourceFiles=("t_discover.c" "t_main.c" "t_network.c" "t_pack.c" "t_parser.c" )
hxaSourceFiles=("hxa_load_save.c" "hxa_util_inflate.c" "hxa_util_png.c" "hxa_util_blur.c")
parallaxSourceFiles=("parallax_controller_testify_util.c" "parallax_controller_io.c" "parallax_controller_compatebility_implemenation.c" "parallax_controller_remote.c" "parallax_controller_merge.c" "parallax_controller_main.c" "parallax_shared_color.c" "parallax_shared_io.c" "parallax_shared_encrypt.c" "parallax_controller_transform.c")

sourceFiles=( "${betraySourceFiles[@]}" "${forgeSourceFiles[@]}" "${imagineSourceFiles[@]}" "${relinquishSourceFiles[@]}" "${seduceSourceFiles[@]}" "${testifySourceFiles[@]}" "${zenithSourceFiles[@]}" "${hxaSourceFiles[@]}" "${parallaxSourceFiles[@]}" )

function echo_error()
{
    echo -e "\033[0;31\x1b[1mERROR:\x1b[0m $1"
}

function echo_success()
{
    echo -e "\033[0;32m\x1b[1mSUCCESS:\x1b[0m $1"
}

function echo_warning()
{
    echo -e "\033[0;33m\x1b[1mWARNING:\x1b[0m $1" 
}

function programIsInstalled()
{
    local programName=$1
    command -v "$programName" &> /dev/null

    if [ $? -eq 0 ]; then
        return $successValue
    fi

    return $errorValue
}

function prequisitesMet()
{
    local programsToInstall=()

    if ! programIsInstalled "clang"; then
        programsToInstall+=("clang")
    fi

    if [ ${#programsToInstall[@]} -eq 0 ]; then
        return $successValue
    fi

    local programList=${programsToInstall[@]}
    echo_error "Missing package(s) \"${programList}\" please install missing package(s)."
    return $errorValue
}

function checkPrerequisites()
{
    echo
    echo "Checking prequisites..."
    if ! prequisitesMet; then
        echo_error "Exiting build script because prequisites are not met"
        exit
    fi
    echo_success "... prerequisites are met!"
}

function getBuildPath()
{
    echo "$scriptPath/build/$buildMode"
}

function getBuildOutputPath()
{
    local buildPath=$(getBuildPath $buildMode)
    echo "$buildPath/$projectName.app"
}

function getBuildArtifactsPath()
{
    local buildPath=$(getBuildPath $buildMode)
    echo "$buildPath/artifacts"
}

function createBuildDirectories()
{
    local buildFolder=$(getBuildPath)
    if ! [ -d $buildFolder ]; then
        mkdir -p $buildFolder
    fi

    local buildArtifactsPath=$(getBuildArtifactsPath)
    if ! [ -d $buildArtifactsPath ]; then
        mkdir -p $buildArtifactsPath
    fi

    local buildOutputPath=$(getBuildOutputPath)
       if ! [ -d $buildOutputPath ]; then
        mkdir -p $buildOutputPath
    fi
}

function printHelpText()
{
    echo "Usage: bash.sh [OPTIONS] [BUILD]"
    echo
    echo "OPTIONS are:"
    echo "-h    Print this help text"
    echo "-s    Skip verification of base prerequisites"
    echo "-i    Install & Run after script finished"
    echo
    echo "BUILD are (release is default):"
    echo "release"
    echo "debug"
    echo
}

function generateInfoPlist()
{
    local buildPath=$(getBuildOutputPath)
    local infoPlistPath="$buildPath/Info.plist"

    echo
    echo "Generating Info.plist..."

    #FK: Remove old manifest
    if [ -f $infoPlistPath ]; then
        rm $infoPlistPath
    fi

    #FK: Create new manifest from template
    if ! cp "$scriptPath/Info.plist.template" $infoPlistPath; then
        echo_error "Couldn't create Info.plist"
        return $errorValue
    fi

    local versionString="$version.$subVersion"

    #FK: Replace template arguments in manifest
    if ! sed -i.bk 's/%MAJORVERSION%/'$version'/' $infoPlistPath; then
        echo_warning "Couldn't find '%MAJORVERSION% template argument in Info.plist file $infoPlistPath"
    fi

    if ! sed -i.bk 's/%MINORVERSION%/'$subVersion'/' $infoPlistPath; then
        echo_warning "Couldn't find '%MINORVERSION% template argument in Info.plist file $infoPlistPath"
    fi

    if ! sed -i.bk 's/%EXECUTABLENAME%/'$executableName'/' $infoPlistPath; then
        echo_warning "Couldn't find '%EXECUTABLENAME% template argument in Info.plist file $manifestPath"
    fi
    
    if ! sed -i.bk 's/%VERSIONSTRING%/'$versionString'/' $infoPlistPath; then
        echo_warning "Couldn't find '%VERSIONSTRING% template argument in Info.plist file $manifestPath"
    fi

    if ! sed -i.bk 's/%BUNDLEID%/'$bundleId'/' $infoPlistPath; then
        echo_warning "Couldn't find '%BUNDLEID% template argument in Info.plist file $manifestPath"
    fi

    if ! sed -i.bk 's/%PROJECTNAME%/'$projectName'/' $infoPlistPath; then
        echo_warning "Couldn't find '%PROJECTNAME% template argument in Info.plist file $manifestPath"
    fi

    #FK: on OSX we have to supply the -i command with a backup name...We don't need a backup so we just delete the backup afterwards..
    rm "$infoPlistPath.bk"

    echo_success "Generated Info.plist!"

    return $successValue
}

function compileUsingMakeFile()
{
    local makeFilePath="$(getBuildArtifactsPath)"
    local makeCommand="make"
    
    pushd $makeFilePath
    eval $makeCommand
    popd

    return $?
}

function generateMakeFile()
{
    local makeFilePath="$(getBuildArtifactsPath)/makefile"
    local objectOutputPath="$(getBuildArtifactsPath)/obj"
    local outputPath="$(getBuildOutputPath)/$executableName"
    local libraries="-lobjc"
    local sysroot="-isysroot $iosSysRoot"
    local frameworks="-framework UiKit -framework CoreFoundation -framework OpenGLES -framework GLKit"
    local compilerArgs="-ferror-limit=900 -Wno-deprecated-declarations -working-directory=${sourceFileDirectory} -fstrict-aliasing ${sysroot}"
    local linkerArgs="${sysroot} ${frameworks} ${libraries}"
    local objectFiles=()

    mkdir -p $objectOutputPath

    if [ "${buildMode}" == "release" ]; then
	    echo "Build config = release"
	    compilerArgs="$compilerArgs -O3"
    fi

    if [ "${buildMode}" == "debug" ]; then
	    echo "Build config = debug"
	    compilerArgs="$compilerArgs --debug"
    fi

    for sourceFile in ${sourceFiles[@]}; do
        local sourceFileLength=${#sourceFile}
        objectFile="${sourceFile:0:${sourceFileLength} - 2}.o"
        objectFiles+="$objectOutputPath/$objectFile "
    done

    echo "CC=clang"                                                 >  $makeFilePath
    echo "CFLAGS=$compilerArgs"                                     >> $makeFilePath
    echo "LDFLAGS=$linkerArgs"                                      >> $makeFilePath
    echo "SRCDIR=$sourceFileDirectory"                              >> $makeFilePath
    echo "OBJDIR=$objectOutputPath"                                 >> $makeFilePath
    echo "EXECUTABLE=$outputPath"                                   >> $makeFilePath
    echo "OBJECTS:=${objectFiles[@]}"                               >> $makeFilePath
    echo "\$(EXECUTABLE): \$(OBJECTS)"                              >> $makeFilePath
    echo $'\t'"\$(CC) \$(LDFLAGS) \$(OBJECTS) -o \$@ "              >> $makeFilePath
    echo "\$(OBJDIR)/%.o: \$(SRCDIR)/%.c"                           >> $makeFilePath
    echo $'\t'"\$(CC) \$(CFLAGS) -c $< -o \$@"                      >> $makeFilePath
    echo "clear:"                                                   >> $makeFilePath
    echo $'\t'"rm -rf $objectOutputPath"                            >> $makeFilePath
}

function copyResourcesToApp()
{
    local outputPath="$(getBuildOutputPath)"
    
    if [ ${#assetsPaths[@]} -gt 0 ]; then
        echo "Copying ios assets to $outputPath..."
        for assetPath in ${assetsPaths[@]}; do
            echo "  ... copying $assetPath"
            cp -rf $assetPath $outputPath/
        done
    fi
}

function main()
{
    #FK: Options:
    #       h - show help
    #       s - skip verification of base prerequisites
    #       i - install & run after compilation
    #       c - skip compilation
    #       d - build for ios device instead of simulation
    while getopts "hsicd" opt; do
        case ${opt} in
            h )
                printHelpText
                justPrintHelp=1
                ;;
            s )
                skipVerification=1
                ;;
            d )
                buildForDevice=1
                ;;
            /? )
                echo "Invalid option: -$OPTARG"
                ;;
        esac
    done
    shift $((OPTIND -1))

    if [ $justPrintHelp -eq 1 ]; then
        exit
    fi

    buildMode=$(checkBuildMode $1)

    if [ $buildForDevice -eq 1 ]; then
        iosSysRoot="$( xcrun --sdk iphoneos --show-sdk-path )"
    fi

    if [ $skipVerification -eq 1 ]; then
        echo "Skipping installation verification..."
    else
        checkPrerequisites
    fi

    echo "Building using build mode '$buildMode'"

    createBuildDirectories
    generateInfoPlist
    generateMakeFile

    if ! compileUsingMakeFile; then
        echo
        echo_error "Compilation failed, aborting apk building process"
        exit
    fi

    copyResourcesToApp

    echo_success "Successfully build app '${projectName}'"
}

main $@
