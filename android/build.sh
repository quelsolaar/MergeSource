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

scriptPath="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

errorValue=1
successValue=0

justPrintHelp=0
skipVerification=0

createAppBundle=0

#FK: Change these at will
#Note: run sdkmanager --list to see what version are available
sdkVersion="31"
ndkVersion="23.1.7779620"
jdkVersion="17"
keystoreFilePath="$scriptPath/betray.keystore"
keystorePassword="test123"

gdbServerPort=5039

buildMode=""

#FK: Valid are "arm64", "arm32", "x86" & "x64"
architectures=()

#FK: These are placeholder credentials
keystoreCredentials="CN=mqttserver.ibm.com, OU=ID, O=IBM, L=Hursley, S=Hants, C=GB"

projectName="betray_app"

activityName="android.app.NativeActivity"
packageName="com.default.betray_app"
libraryName="betray_app.so"
iconName="betray"
version="14"
subVersion="0"

resourcePath="$scriptPath/res"
assetsPaths=()

sourceFileDirectory="$scriptPath/../"

librariesToLinkAgainst=("-lm" "-lGLESv2" "-lc" "-landroid" "-lEGL" "-llog")

betraySourceFiles=("b_main.c" "b_android_main.c" "b_android_key_codes.c")
forgeSourceFiles=("f_math.c" "f_matrix_make.c" "f_matrix_operations.c" "f_mem_debug.c" "f_noise.c" "f_path.c" "f_sort.c" "f_text.c" "f_types.c")
imagineSourceFiles=("i_dir.c" "i_library.c" "i_settings.c" "i_threads.c" "i_time.c")
relinquishSourceFiles=("r_draw_buffer.c" "r_draw_extensions.c" "r_draw_framebuffer.c" "r_draw_parser.c" "r_draw_primitive.c" "r_draw_shader.c" "r_draw_state.c" "r_draw_uniform.c" "r_matrix.c" )
seduceSourceFiles=("s_background.c" "s_background_old.c" "s_draw_3d.c" "s_font_abel.c" "s_font_arial.c" "s_font_eurasia.c" "s_font_impact.c" "s_font_lane.c" "s_font_lato.c" "s_font_luke.c" "s_font_oswald.c" "s_font_sansumi.c" "s_font_times.c" "s_font_verdana.c" "s_lens.c" "s_lines.c" "s_main.c" "s_manipulator_point.c" "s_manipulator_rotate.c" "s_manipulator_scale.c" "s_manipulator_space.c" "s_particles.c" "s_popup.c" "s_popup_detect.c" "s_projection.c" "s_select.c" "s_settings_window.c" "s_settings_window2.c" "s_sort.c" "s_text.c" "s_text_edit.c" "s_text_exporter.c" "s_text_file_format.c" "s_text_output.c" "s_text_widget.c" "s_translate.c" "s_widget.c" "s_widget_radial.c" "s_widget_visualizers.c")
testifySourceFiles=("t_discover.c" "t_main.c" "t_network.c" "t_pack.c" "t_parser.c" )
hxaSourceFiles=("hxa_load_save.c" "hxa_util_inflate.c" "hxa_util_png.c" "hxa_util_blur.c")
parallaxSourceFiles=("parallax_controller_testify_util.c" "parallax_controller_io.c" "parallax_controller_compatebility_implemenation.c" "parallax_controller_remote.c" "parallax_controller_merge.c" "parallax_controller_main.c" "parallax_shared_color.c" "parallax_shared_io.c" "parallax_shared_encrypt.c" "parallax_controller_transform.c")
androidNativeAppGlueSourceFiles=("b_android_native_app_glue.c")

sourceFiles=( "${betraySourceFiles[@]}" "${forgeSourceFiles[@]}" "${imagineSourceFiles[@]}" "${relinquishSourceFiles[@]}" "${seduceSourceFiles[@]}" "${testifySourceFiles[@]}" "${hxaSourceFiles[@]}" "${parallaxSourceFiles[@]}" "${androidNativeAppGlueSourceFiles[@]}" )

function echo_error()
{
    echo -e "\033[0;31m\e[1mERROR:\e[0m $1"
}

function echo_success()
{
    echo -e "\033[0;32m\e[1mSUCCESS:\e[0m $1"
}

function echo_warning()
{
    echo -e "\033[0;33m\e[1mWARNING:\e[0m $1" 
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

function isJDKInstalled()
{
    if ! programIsInstalled "javac"; then 
        return $errorValue
    fi

    local javacVersion="$(javac --version)"

    #FK: We don't care what subversion, only get the major version (eg: "javac 17" instead of "javac 17.0.1")
    local javacVersionShortened=${javacVersion:0:8}
    local expectedVersion="javac $jdkVersion"

    if [ "$javacVersionShortened" != "$expectedVersion" ]; then
        return $errorValue
    fi

    return $successValue
}

function isBundleToolInstalled()
{
    local sdkPath=$(getAndroidSDKPath)
    local bundleToolPath="$sdkPath/bundletool"

    if ! [ -f $bundleToolPath ]; then
        return $errorValue
    fi

    return $successValue
}

function prequisitesMet()
{
    local user=$(whoami)
    if [ $user == "root" ]; then
        echo_warning "   Please don't run this script as root."
        return $errorValue
    fi

    local programsToInstall=()

    if ! programIsInstalled "apt-get"; then
        programsToInstall+=("apt-get")
    fi

    if ! programIsInstalled "wget"; then
        programsToInstall+=("wget")
    fi

    if ! programIsInstalled "zip"; then
        programsToInstall+=("zip")
    fi

    if ! programIsInstalled "unzip"; then
        programsToInstall+=("unzip")
    fi

    if ! programIsInstalled "cat"; then
        programsToInstall+=("cat")
    fi

    if ! isJDKInstalled; then
        local jdkPackageName="openjdk-$jdkVersion-jdk-headless"
        programsToInstall+=($jdkPackageName)
    fi

    if [ ${#programsToInstall[@]} -eq 0 ]; then
        return $successValue
    fi

    local programList=${programsToInstall[@]}
    echo_error "Missing package(s) \"${programList}\" please run \"sudo apt-get install ${programList}\""
    return $errorValue
}

function getAndroidSDKPath()
{
    #FK: This path is the users home directory
    #    TODO: Check if there is a 'default' path for the android sdk
    local sdkPath="$(echo ~)/.android_sdk_$sdkVersion"
    echo $sdkPath
}

function getAndroidSDKCommandLineToolsPath()
{
    local sdkPath=$(getAndroidSDKPath $sdkVersion)
    local sdkCommandLineToolPath="$sdkPath/cmdline-tools"

    echo $sdkCommandLineToolPath
}

function getAndroidSDKCommandLineToolApplicationPath()
{
    local sdkCommandLineToolPath=$(getAndroidSDKCommandLineToolsPath $sdkVersion)
    local sdkManagerApplicationPath="$sdkPath/cmdline-tools/bin/sdkmanager"

    echo $sdkManagerApplicationPath
}

function getAndroidSDKPlatformPath()
{
    local sdkPath=$(getAndroidSDKPath)
    local sdkPlatformPath="$sdkPath/platforms/android-$sdkVersion"

    echo $sdkPlatformPath
}

function getAndroidSDKBuildToolsPath()
{
    local sdkPath=$(getAndroidSDKPath)
    local sdkBuildToolsPath="$sdkPath/build-tools/$sdkVersion.0.0"

    echo $sdkBuildToolsPath
}

function getAndroidSDKPlatformToolsPath()
{
    local sdkPath=$(getAndroidSDKPath)
    local sdkPlatformToolsPath="$sdkPath/platform-tools"

    echo $sdkPlatformToolsPath
}

function getAndroidNDKPath()
{
    local sdkPath=$(getAndroidSDKPath)
    local ndkPath="$sdkPath/ndk/$ndkVersion"

    echo $ndkPath
}

function isAndroidSDKManagerInstalled()
{
    local sdkCommandlineToolsPath=$(getAndroidSDKCommandLineToolsPath)
    local sdkManagerPath="$sdkCommandlineToolsPath/bin/sdkmanager"

    if programIsInstalled $sdkManagerPath; then
        return $successValue
    fi

    return $errorValue
}

function installAndroidSDKManager()
{
    local sdkPath=$(getAndroidSDKPath)

    if ! [ -d $sdkPath ]; then
        local createSDKPathCommand="mkdir -p $sdkPath"
        if ! eval $createSDKPathCommand; then
            echo_error "Couldn't create sdk path '$sdkPath'"
            return $errorValue
        fi
    fi


    #FK: Change link at will. It used to be tied to the sdk version but apparently that changed :(
    local sdkCommandLineDownloadArchiveName="commandlinetools-linux-7583922_latest.zip"
    local sdkDownloadPath="https://dl.google.com/android/repository/$sdkCommandLineDownloadArchiveName"
    local wgetSDKDownloadCommand="wget -o $sdkPath/$sdkCommandLineDownloadArchiveName $sdkDownloadPath"

    echo "   Downloading android sdk command line tools for sdk version $sdkVersion..."
    if ! eval $wgetSDKDownloadCommand; then
        echo_error "   Couldn't download android sdk commandline tool using command '$wgetSDKDownloadCommand'"
        return $errorValue
    fi

    echo_success "   Downloaded android sdk commandline tool archive '$sdkCommandLineDownloadArchiveName"
    echo "   Trying to export android sdk commandline tool archive to $sdkPath..."

    local sdkArchiveExportCommand="unzip -o -q $sdkCommandLineDownloadArchiveName -d $sdkPath"
    if ! eval $sdkArchiveExportCommand; then
        echo_error "   Couldn't extract android sdk commandline tool archive using command '$sdkArchiveExportCommand'"
        return $errorValue
    fi

    echo_success "   Finished exporting the android sdk commandline tool archive."
    
    echo "   Accepting SDK licenses..."
    local sdkManagerPath=$(getAndroidSDKCommandLineToolApplicationPath $sdkVersion)
    local sdkLicenseAgreementCommand="yes | $sdkManagerPath --sdk_root=$sdkPath --licenses &> /dev/null"
    eval $sdkLicenseAgreementCommand
    
    return $successValue
}

function isAndroidSDKInstalled()
{
    local sdkPlatformPath=$(getAndroidSDKPlatformPath)
    if ! [ -d $sdkPath ]; then

        return $errorValue
    fi

    local ndkPath=$(getAndroidNDKPath)
    if ! [ -d $ndkPath ]; then
        return $errorValue
    fi

    local sdkBuildTools=$(getAndroidSDKBuildToolsPath)
    if ! [ -d $sdkBuildTools ]; then
        return $errorValue
    fi

    return $successValue
}

function installAndroidSDK()
{
    local buildToolsVersion=$sdkVersion.0.0

    local sdkPath=$(getAndroidSDKPath)
    local sdkManager=$(getAndroidSDKCommandLineToolApplicationPath)
    local installSDKPlatformCommand=" $sdkManager --sdk_root=$sdkPath 'platforms;android-$sdkVersion'"
    local installNDKCommand="$sdkManager --sdk_root=$sdkPath 'ndk;$ndkVersion'"
    local installBuildToolsCommand=" $sdkManager --sdk_root=$sdkPath 'build-tools;$buildToolsVersion'"

    echo "   ... installing sdk platform $sdkVersion"
    if ! eval $installSDKPlatformCommand; then
        return $errorValue
    fi

    echo
    echo "   ... installing ndk $ndkVersion"
    if ! eval $installNDKCommand; then
        return $errorValue
    fi

    echo
    echo "   ... installing build-tools $buildToolsVersion"
    if ! eval $installBuildToolsCommand; then
        return $errorValue
    fi

    return $successValue
}

function installBundleTool()
{
    local sdkPath=$(getAndroidSDKPath)
    local bundleToolLink="https://github.com/google/bundletool/releases/download/1.9.1/bundletool-all-1.9.1.jar"
    local bundleToolPath="$sdkPath/bundletool"
    local downloadBundleToolCmd="wget -O $bundleToolPath $bundleToolLink"

    echo "   ... downloading bundletool..."
    if ! eval $downloadBundleToolCmd; then
        echo_error "   Couldn't download bundletoolk commandline tool using command '$downloadBundleToolCmd'"
        return $errorValue
    fi

    echo_success "   successfully downloaded bundletool to $bundleToolPath"
    return $successValue
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

function checkAndroidSDKManagerInstallation()
{
    echo
    echo "Check if Android SDK Manager $sdkVersion is installed..."
    if ! isAndroidSDKManagerInstalled $sdkVersion; then
        echo "   Couldn't find Android SDK Manager $sdkVersion installation"
        if ! installAndroidSDKManager $sdkVersion; then
            echo_error "Exiting build script because Android SDK Manager couldn't get installed"
            exit
        fi
    fi
    echo_success "... SDK Manager is installed!"
}

function checkAndroidSDKInstallation()
{
    echo
    echo "Check if Android SDK $sdkVersion and NDK $ndkVersion are installed..."
    if ! isAndroidSDKInstalled $sdkVersion $ndkVersion; then
        echo "   Couldn't find Android SDK $sdkVersion or NDK $ndkVersion installation"
        if ! installAndroidSDK $sdkVersion $ndkVersion; then
            echo_error "Exiting build script because Android SDK $sdkVersion coulnd't get installed"
            exit
        fi
    fi
    echo_success "... SDK $sdkVersion and NDK $ndkVersion are installed!"
}

function checkBundleToolInstallation()
{
    echo
    echo "Check if bundletool is installed..."
    if ! isBundleToolInstalled; then
        echo "   Couldn't find bundletool installation"
        if ! installBundleTool;  then
            echo_error "Exiting build script because bundletool coulnd't get installed"
            exit
        fi
    fi
    echo_success "...bundletool is installed!"
}

function checkJDKInstallation()
{
    echo
    echo "Check if JDK $jdkVersion is installed..."
    if ! isJDKInstalled; then 
        echo "   Couldn't find JDK $jdkVersion installation"
        if ! installJDK; then 
            echo_error "Exiting build script because JDK couldn't get installed"
            exit
        fi
    fi
    echo_success "... JDK $jdkVersion is installed!"
}

function getAndroidNDKToolChainsPath()
{
    local ndkPath=$(getAndroidNDKPath)
    local toolchainsPath="$ndkPath/toolchains/llvm/prebuilt/linux-x86_64/bin"

    echo $toolchainsPath
}

function getBuildPath()
{
    echo "$scriptPath/build/$buildMode"
}

function getBuildArtifactsPath()
{
    local buildPath=$(getBuildPath $buildMode)
    echo "$buildPath/artifacts"
}

function getBuildApkPackagePath()
{
    local buildPath=$(getBuildPath $buildMode)
    echo "$buildPath/apk_package"
}

function getBuildAppBundlePath()
{
    local buildPath=$(getBuildPath $buildMode)
    echo "$buildPath/app_bundle"
}

function getAndroidNDKSysrootPath()
{
    local ndkPath=$(getAndroidNDKPath)
    local sysrootPath="$ndkPath/toolchains/llvm/prebuilt/linux-x86_64/sysroot"

    echo $sysrootPath
}

function getAndroidNDKIncludePath()
{
    local ndkSysrootPath=$(getAndroidNDKSysrootPath)
    local ndkIncludePath="$ndkSysrootPath/usr/include"

    echo $ndkIncludePath
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

    mkdir -p "$buildArtifactsPath/jni"
    mkdir -p "$buildArtifactsPath/compiled_resources"
    mkdir -p "$buildArtifactsPath/unpacked_apk"

    local buildApkPackagePath=$(getBuildApkPackagePath)
    rm -rf $buildApkPackagePath
    mkdir -p $buildApkPackagePath

    local buildAppBundlePath=$(getBuildAppBundlePath)
    rm -rf $buildAppBundlePath
    mkdir -p $buildAppBundlePath
    mkdir -p "$buildAppBundlePath/manifest"
    mkdir -p "$buildAppBundlePath/res"
}

function generateAndroidManifest()
{
    local buildPath=$(getBuildArtifactsPath)
    local manifestPath="$buildPath/AndroidManifest.xml"

    echo
    echo "Generating Android manifest..."

    #FK: Remove old manifest
    if [ -f $manifestPath ]; then
        rm $manifestPath
    fi

    #FK: Create new manifest from template
    if ! cp "$scriptPath/AndroidManifest.xml.template" $manifestPath; then
        echo_error "Couldn't create Android Manifest"
        return $errorValue
    fi

    local debuggable="false"
    if [ $buildMode == "debug" ]; then
        debuggable="true"
    fi

    local versionName="$version.$subVersion"

    #FK: Replace template arguments in manifest
    if ! sed -i 's/%VERSIONCODE%/'$version'/' $manifestPath; then
        echo_warning "Couldn't find '%VERSIONCODE% template argument in Android Manifest file $manifestPath"
    fi

    if ! sed -i 's/%VERSIONNAME%/'$versionName'/' $manifestPath; then
        echo_warning "Couldn't find '%VERSIONNAME% template argument in Android Manifest file $manifestPath"
    fi

    if ! sed -i 's/%PACKAGENAME%/'$packageName'/' $manifestPath; then
        echo_warning "Couldn't find '%PACKAGENAME% template argument in Android Manifest file $manifestPath"
    fi

    if ! sed -i 's/%SDKVERSION%/'$sdkVersion'/' $manifestPath; then
        echo_warning "Couldn't find '%SDKVERSION% template argument in Android Manifest file $manifestPath"
    fi

    if ! sed -i 's/%LIBRARYNAME%/'$projectName'/' $manifestPath; then
        echo_warning "Couldn't find '%LIBRARYNAME% template argument in Android Manifest file $manifestPath"
    fi

    if ! sed -i 's/%ICONNAME%/'$iconName'/' $manifestPath; then
        echo_warning "Couldn't find '%ICONNAME% template argument in Android Manifest file $manifestPath"
    fi

    if ! sed -i 's/%LABEL%/'$projectName'/' $manifestPath; then
        echo_warning "Couldn't find '%ICONNAME% template argument in Android Manifest file $manifestPath"
    fi

    if ! sed -i 's/%ACTIVITYNAME%/'$activityName'/' $manifestPath; then
        echo_warning "Couldn't find '%ACTIVITYNAME% template argument in Android Manifest file $manifestPath"
    fi

    if ! sed -i 's/%DEBUGGABLE%/'$debuggable'/' $manifestPath; then
        echo_warning "Couldn't find %DEBUGGABLE% template argument in Android Manifest file $manifestPath"
    fi

    echo_success "Generated Android manifest!"

    return $successValue
}

function generateApk()
{
    local buildPath=$(getBuildPath)
    local buildArtifactsPath=$(getBuildArtifactsPath)
    local buildPackagePath=$(getBuildApkPackagePath)

    local buildPackageManifestPath="$buildArtifactsPath/AndroidManifest.xml"
    local unalignedApkPath="$buildArtifactsPath/$projectName.unaligned.apk"
    local alignedApkPath="$buildPath/$projectName.apk"

    local sdkBuildToolsPath=$(getAndroidSDKBuildToolsPath)
    local sdkPlatformPath=$(getAndroidSDKPlatformPath)
    local androidJarPath="$sdkPlatformPath/android.jar"

    #FK: Copy libraries to build package path to be able to add them to the apk
    local copyLibCmd="cp -rf $buildArtifactsPath/libs $buildPackagePath/lib"
    if ! eval $copyLibCmd; then
        echo_error "Couldn't copy native libraries to apk."
        return $errorValue
    fi

    local createUnalignedApkCommand="$sdkBuildToolsPath/aapt p -f -M $buildPackageManifestPath -F $unalignedApkPath -I $androidJarPath"
    if [ "$resourcePath" != "" ]; then
        createUnalignedApkCommand+=" -S $resourcePath"
    fi

    if [ -d $scriptPath/assets ]; then
        rm -rf $scriptPath/assets
    fi

    if [ ${#assetsPaths[@]} -gt 0 ]; then
        mkdir $scriptPath/assets

        echo "Copying android assets to $scriptPath/assets..."
        for assetPath in ${assetsPaths[@]}; do
            echo "  ... copying $assetPath"
            cp -rf $assetPath $scriptPath/assets
        done

        createUnalignedApkCommand+=" -A $scriptPath/assets"
    fi

    createUnalignedApkCommand+=" $buildPackagePath"
    local alignApkCommand="$sdkBuildToolsPath/zipalign -p -f -v 4 $unalignedApkPath $alignedApkPath"
    local signApkCommand="$sdkBuildToolsPath/apksigner sign --ks $keystoreFilePath --ks-pass pass:$keystorePassword $alignedApkPath"

    echo
    echo "Creating unaligned apk..."
    if ! eval $createUnalignedApkCommand; then
        echo_error "Couldn't create unaligned apk"
        return $errorValue
    fi
    echo_success "Created unaligned apk!"

    echo
    echo "Adding native library to apk..."
    if ! eval $addLibraryToApkCommand; then
        echo_error "Couldn't add native library to apk"
        return $errorValue
    fi
    echo_success "Added native library to apk!"

    echo
    echo "Aligning apk..."
    if ! eval $alignApkCommand; then
        echo_error "Couldn't align apk"
        return $errorValue
    fi
    echo_success "Aligned apk!"

    echo
    echo "Signing apk using '$keystoreFilePath'..."
    if ! eval $signApkCommand; then
        echo_error "Couldn't sign apk"
        return $errorValue
    fi
    echo_success "Signed apk!"

    cd $scriptPath

    return $successValue
}

function generateAppBundle()
{
    local buildPath=$(getBuildPath)
    local buildArtifactsPath=$(getBuildArtifactsPath)
    local buildPackagePath=$(getBuildAppBundlePath)

    local buildPackageManifestPath="$buildArtifactsPath/AndroidManifest.xml"
    local unalignedApkPath="$buildArtifactsPath/$projectName.unaligned.apk"
    local appBundlePath="$buildPath/$projectName.aab"

    local sdkBuildToolsPath=$(getAndroidSDKBuildToolsPath)
    local sdkPlatformPath=$(getAndroidSDKPlatformPath)
    local androidJarPath="$sdkPlatformPath/android.jar"

    #FK: remove old lib folder in app bundle
    rm -rf "$buildPackagePath/lib"

    #FK: Copy libraries to build package path to be able to add them to the apk
    echo "Adding native library to app bundle..."
    local copyLibCmd="cp -rf $buildArtifactsPath/libs $buildPackagePath/lib"
    if ! eval $copyLibCmd; then
        echo_error "Couldn't copy native libraries to app bundle."
        return $errorValue
    fi

    local aaptPath="$sdkBuildToolsPath/aapt2"
    local resourceFlatFileOutputPath="$buildArtifactsPath/compiled_resources/"

    if [ "$resourcePath" != "" ]; then
        local resourceFiles=$(find $resourcePath -type f)

        for resourceFile in ${resourceFiles[@]}; do
            local createResourceFlatFile="$aaptPath compile $resourceFile -o $resourceFlatFileOutputPath"
            if ! $createResourceFlatFile; then
                echo_error "Couldn't compile resource $resourceFile using aapt2"
                return $errorValue
            fi
        done
    fi

    local createUnalignedApkCommand="$aaptPath link --proto-format -o $unalignedApkPath --manifest $buildPackageManifestPath -I $androidJarPath"
    
    if [ "$resourcePath" != "" ]; then
        createUnalignedApkCommand+=" -R $resourceFlatFileOutputPath/*.flat"
    fi

    local signAppBundleCommand="jarsigner -keystore $keystoreFilePath -storepass $keystorePassword $appBundlePath mykey"

    echo
    echo "Creating unaligned apk..."
    echo $createUnalignedApkCommand
    if ! eval $createUnalignedApkCommand; then
        echo_error "Couldn't create unaligned apk"
        return $errorValue
    fi
    echo_success "Created unaligned apk!"

    local unpackedApkPath="$buildArtifactsPath/unpacked_apk"
    local unpackApkCommand="unzip -o $unalignedApkPath -d $unpackedApkPath"

    echo $unpackApkCommand
    if ! eval $unpackApkCommand; then
        echo_error "couldn't unpack apk using unzip"
        return $errorValue
    fi

    local bundleManifestPath="$buildPackagePath/manifest/AndroidManifest.xml"
    local bundleResourcePath="$buildPackagePath/res"
    
    echo "Copying relevant apk files to $buildPackagePath..."
    cp -f $unpackedApkPath/AndroidManifest.xml $bundleManifestPath
    cp -f $unpackedApkPath/resources.pb $buildPackagePath
    cp -rf $scriptPath/assets $buildPackagePath
    cp -rf $unpackedApkPath/res $buildPackagePath

    echo "zip content of $buildPackagePath..."
    local bundleZipPath="$buildArtifactsPath/bundle.zip"
    local zipCommand="pushd $buildPackagePath && zip -r $bundleZipPath ./ && popd"

    #FK: remove old zip path
    rm -f $bundleZipPath

    if ! eval $zipCommand; then
        echo_error "Couldn't zip folder $buildPackagePath..."
        return $errorValue
    fi

    echo "create bundle using bundletool..."
    local sdkPath=$(getAndroidSDKPath)
    local bundleToolPath="$sdkPath/bundletool"
    local bundleToolCmd="java -jar $bundleToolPath build-bundle --modules=$bundleZipPath --output=$appBundlePath"
    
    #FK: Delete old app bundle
    rm -f $appBundlePath

    eval $bundleToolCmd

    echo
    echo "Signing bundle using '$keystoreFilePath'..."
    echo $signAppBundleCommand
    if ! eval $signAppBundleCommand; then
        echo_error "Couldn't sign app bundle using '$signAppBundleCommand'"
        return $errorValue
    fi
    echo_success "Signed app bundle!"

    return $successValue
}

function checkKeystoreFile()
{
    if ! [ -f $keystoreFilePath ]; then
        echo "Keystore file $keystoreFilePath doesn't exist."
        echo "Do you want to create a keystore file (y/n)?"
        local result=""
        read -n 1 result
        if [ "$result" == "n" ]; then
            echo "Didn't create keystore file, please provide a keystore file manually to $keystoreFilePath"
            return $errorValue
        else 
            if [ "$result" == "y" ]; then
                local keytoolCommand="keytool -validity 20000 -keystore $keystoreFilePath -genkey -noprompt -keypass $keystorePassword -storepass $keystorePassword -keyalg RSA -dname \"$keystoreCredentials\""
                echo
                echo "Creating keystore file with '$keytoolCommand'..."
                if ! eval $keytoolCommand; then
                    echo_error "Failed to create keystore file. Please provide a keystore file manually to $keystoreFilePath"
                    return $errorValue
                fi
                echo_success "Created keystore file at $keystoreFilePath!"
                return $successValue
            fi
        fi
    fi

    return $successValue
}

function printHelpText()
{
    echo "Usage: bash.sh [OPTIONS] [BUILD] [ARCH]"
    echo
    echo "OPTIONS are:"
    echo "-h    Print this help text"
    echo "-s    Skip verification of base prerequisites (android sdk installation, etc)"
    echo "-i    Install & Run after script finished"
    echo
    echo "BUILD are (release is default):"
    echo "release"
    echo "debug"
    echo
    echo "ARCH are (arm64 is default):"
    echo "arm64"
    echo "arm32"
    echo "x32"
    echo "x64"
}

function getPositionOfFirstOccurance()
{
    #https://stackoverflow.com/questions/5031764/position-of-a-string-within-a-string-using-linux-shell-script
    local x="${1%%$2*}"
    [[ "$x" = "$1" ]] && echo -1 || echo "${#x}"
}

function installAndRunApk()
{
    local apkPath=$1
    local sdkPath=$(getAndroidSDKPlatformToolsPath)
    local adbPath="$sdkPath/adb"

    local installApkCommand="$adbPath install -r $apkPath"
    local runApkCommand="$adbPath shell am start -n $packageName/$activityName"

    echo
    echo "Installing apk $packageName on device..."
    if ! eval $installApkCommand; then
        echo_error "Couldn't install apk using command \"$installApkCommand\""
        return $errorValue
    fi
    echo_success "Successfully installed apk on device"

    echo
    echo "Trying to start app $packageName on device..."
    if ! eval $runApkCommand; then
        echo_error "Couldn't start apk $packageName using command \"$runApkCommand\""
        return $errorValue
    fi
    echo_success "Successfully started app $packageName on device"

    return $successValue
}

function getAndroidDeviceName()
{
    local sdkVersion=$1
    local sdkPath=$(getAndroidSDKPlatformToolsPath)
    local adbPath="$sdkPath/adb"

    eval "$adbPath devices -l | tail -n 2 | awk '{ print \$1 }' | xargs"
}

function startDebugServer()
{
    local sdkPath=$(getAndroidSDKPlatformToolsPath)
    local adbPath="$sdkPath/adb"

    echo
    echo "Killing all processes from app $packageName on device..."
    local remotePsCommand="$adbPath shell run-as $packageName ps -ef | awk '{ print \$2 }'"
    local appPids=( $($remotePsCommand) )

    #FK: Remove first entry (which will be "PID")
    appPids="${appPids[@]:1}"

    for appPid in ${appPids[@]}; do
        local killProcessCommand="1adbPath shell run-as $packageName kill $appPid"
        eval $killProcessCommand 2> /dev/null
    done

    echo
    echo "Checking if process application from devices has already been pulled..."
    local buildPath=$(getBuildPath)
    local deviceName=$(getAndroidDeviceName)
    
    local requiredFiles=()

    local localSysrootPath="$buildPath/$deviceName/sysroot"


    if [ $architecture == "arm64" ]; then
        mkdir -p "$localSysrootPath/system/bin"
        mkdir -p "$localSysrootPath/system/lib64"
        requiredFiles+=("/system/bin/app_process64")
        requiredFiles+=("/system/bin/linker64")
        requiredFiles+=("/system/lib64/libc.so")
        requiredFiles+=("/system/lib64/libm.so")
        requiredFiles+=("/system/lib64/libdl.so")
    else
        mkdir -p "$localSysrootPath/system/bin"
        mkdir -p "$localSysrootPath/system/lib"
        requiredFiles+=("/system/bin/app_process")
        requiredFiles+=("/system/bin/linker")
        requiredFiles+=("/system/lib/libc.so")
        requiredFiles+=("/system/lib/libm.so")
        requiredFiles+=("/system/lib/libdl.so")
    fi

    for requiredFile in ${requiredFiles[@]}; do
        eval "$adbPath pull $requiredFile $localSysrootPath/$requiredFile"
    done

    echo
    echo "Trying to query process id of app $packageName on device..."
    local appPid=""
    local attemptCount=1
    while [ "$appPid" == "" -o $attemptCount -eq 10 ]; do
        echo "   Attempt $attemptCount..."
        appPid="$($adbPath shell pidof $packageName)"
        attemptCount=$(($attemptCount + 1))
    done
    echo_success "Found process id of app $packageName on device! (AppId: $appPid)"

    if [ $appPid == "" ]; then
        echo_error "Could not query process if after $attemptCount tries..."
        return $errorValue
    fi

    echo
    echo "Forwarding lldb-server port $gdbServerPort..."
    local portForwardingCommand="$adbPath forward tcp:$gdbServerPort tcp:$gdbServerPort"
    eval $portForwardingCommand

    echo
    echo "Trying to start lldb-server on device..."
    local apkPathCommand="$adbPath shell pm path $packageName"
    local apkPathCommandResult=$(eval $apkPathCommand)
    local delimiterPos=$(getPositionOfFirstOccurance $apkPathCommandResult ":")
    local apkBasePath="${apkPathCommandResult:$delimiterPos+1}"
    local apkPath="${apkBasePath:0:-9}"
    local debugServerPath="$apkPath/lib/$architecture/lldb-server"
    local debugServerStartCommand="$adbPath shell run-as $packageName $debugServerPath gdbserver unix://$gdbServerPort --attach $appPid"
    echo $debugServerStartCommand
    #eval $debugServerStartCommand
}

function convertArchitectureToLLVMArchitecture()
{
    local architecture=$1

    if [ $architecture == "arm32" ]; then
        echo "armeabi-v7a"
    elif [ $architecture == "arm64" ]; then
        echo "arm64-v8a"
    elif [ $architecture == "x86" ]; then
        echo "x86"
    elif [ $architecture == "x64" ]; then
        echo "x86_64"
    fi
}

function convertArchitectureToLLDBArchitecture()
{
    local architecture=$1

    if [ $architecture == "arm32" ]; then
        echo "arm"
    elif [ $architecture == "arm64" ]; then
        echo "aarch64"
    elif [ $architecture == "x86" ]; then
        echo "i386"
    elif [ $architecture == "x64" ]; then
        echo "x86_64"
    fi
}

function generateMakeFiles()
{
    local buildArtifactsFolder=$(getBuildArtifactsPath)
    local appMakeFilePath="$buildArtifactsFolder/jni/Application.mk"
    local androidMakeFilePath="$buildArtifactsFolder/jni/Android.mk"
    local manifestFilePath="$buildArtifactsFolder/AndroidManifest.xml"

    local sourceFilePaths=""
    local architecturesToCompileFor=""

    for sourceFile in ${sourceFiles[@]}; do
        sourceFilePath+="$sourceFileDirectory/$sourceFile "
    done

    for architecture in ${architectures[@]}; do
        architecturesToCompileFor+="$(convertArchitectureToLLVMArchitecture $architecture) "
    done

    local tempAppMakeFilePath="$appMakeFilePath.temp"
    local tempAndroidMakeFilePath="$androidMakeFilePath.temp"

    echo "LOCAL_PATH := \$(call my-dir)"                          > $tempAndroidMakeFilePath
    echo "include \$(CLEAR_VARS)"                                >> $tempAndroidMakeFilePath
    echo "LOCAL_MODULE := $projectName"                          >> $tempAndroidMakeFilePath
    echo "LOCAL_SRC_FILES := $sourceFilePath"                    >> $tempAndroidMakeFilePath
    echo "LOCAL_LDLIBS := ${librariesToLinkAgainst[@]}"          >> $tempAndroidMakeFilePath
    echo "LOCAL_C_INCLUDES := $sourceFileDirectory"              >> $tempAndroidMakeFilePath
    echo "LOCAL_DISABLE_FORMAT_STRING_CHECKS := true"            >> $tempAndroidMakeFilePath
    echo "include \$(BUILD_SHARED_LIBRARY)"                      >> $tempAndroidMakeFilePath

    
    echo "APP_ABI := $architecturesToCompileFor"     > $tempAppMakeFilePath
    echo "APP_PLATFORM := android-$sdkVersion"      >> $tempAppMakeFilePath
    echo "APP_BUILD_SCRIPT := $androidMakeFilePath" >> $tempAppMakeFilePath
    echo "APP_MANIFEST := $manifestFilePath"        >> $tempAppMakeFilePath
    echo "APP_PROJECT_PATH := $scriptPath"          >> $tempAppMakeFilePath
    echo "APP_MODULES := $projectName"              >> $tempAppMakeFilePath


    local appMakeFileHashPath="$appMakeFilePath.hash"
    local androidMakeFileHashPath="$androidMakeFilePath.hash"

    local generatedAppMakeFileHash=$(sha1sum $tempAppMakeFilePath | awk '{ print $1 }')
    local generatedAndroidMakeFileHash=$(sha1sum $tempAndroidMakeFilePath | awk '{ print $1 }')

    local readAppMakeFileHash=$(cat $appMakeFileHashPath)
    local readAndroidMakeFileHash=$(cat $androidMakeFileHashPath)

    if [ "$generatedAppMakeFileHash" != "$readAppMakeFileHash" ]; then
        echo $generatedAppMakeFileHash > $appMakeFileHashPath
        mv -f $tempAppMakeFilePath $appMakeFilePath
    else
        rm -f $tempAppMakeFilePath
    fi

    if [ "$generatedAndroidMakeFileHash" != "$readAndroidMakeFileHash" ]; then
        echo $generatedAndroidMakeFileHash > $androidMakeFileHashPath
        mv -f $tempAndroidMakeFilePath $androidMakeFilePath
    else
        rm -f $tempAndroidMakeFilePath
    fi
}

function compileUsingMakeFiles()
{
    local ndkPath=$(getAndroidNDKPath)
    local ndkBuildPath="$ndkPath/ndk-build"

    local buildArtifactsPath=$(getBuildArtifactsPath)

    local compileInParallel=1
    local ndkBuildOptions="NDK_PROJECT_PATH=$buildArtifactsPath NDK_APPLICATION_MK=$buildArtifactsPath/jni/Application.mk"

    if [ $buildMode == "debug" ]; then
        ndkBuildOptions+=" NDK_DEBUG=1"
    else
        ndkBuildOptions+=" NDK_DEBUG=0"
    fi

    if [ $compileInParallel -eq 1 ]; then
        ndkBuildOptions+=" -j --output-sync"
    fi

    local ndkBuildCommand="$ndkBuildPath $ndkBuildOptions"
    echo $ndkBuildCommand
    $ndkBuildCommand
    
    return $?
}

function evaluateArchitectures()
{
    for architecture in ${architectures[@]}; do
        local llvmArchitecture=$(convertArchitectureToLLVMArchitecture $architecture)
        if [ "$llvmArchitecture" == "" ]; then
            return $errorValue
        fi
    done

    return $successValue
}

function generateDebugSymbols()
{
    local buildArtifactsPath=$(getBuildArtifactsPath)
    local buildPath=$(getBuildPath)

    local symbolsPath="$buildArtifactsPath/obj/local"
    local zipPath="$buildPath/debug_symbols.zip"
    echo "Trying to create debug symbols..."

    local zipCommand="pushd $symbolsPath && zip -r $zipPath ./ -i *.so && popd"
    echo $zipCommand
    if ! eval $zipCommand; then
        echo_error "Error while trying to compress debug symbols using '$zipCommand'"
        return $errorValue
    fi

    return $successValue
}

function generateDeployScripts()
{
    local apkPath=$1
    
    local ndkPath=$(getAndroidNDKPath)
    local sdkPath=$(getAndroidSDKPlatformToolsPath)
    local buildArtifactsPath=$(getBuildArtifactsPath)
    local buildPath=$(getBuildPath)
    local adbPath="$sdkPath/adb"

    local clearApkDataCommand="$adbPath shell pm clear $packageName"
    local installApkCommand="$adbPath install -r $apkPath"
    local runApkCommand="$adbPath shell am start -n $packageName/$activityName"
    local runDebuggerCommand="$ndkPath/ndk-lldb --adb=$adbPath --project=$buildArtifactsPath --launch -v"

    local deployScriptFileName="deploy_apk.sh"
    local runScriptFileName="run_apk.sh"
    local debugScriptFileName="debug_apk.sh"
    local deployAndRunScriptFileName="deploy_and_run_apk.sh"
    local deployAndDebugScriptFileName="deploy_and_debug_apk.sh"
    
    local deployScriptPath="$buildPath/$deployScriptFileName"
    local runScriptPath="$buildPath/$runScriptFileName"
    local debugScriptPath="$buildPath/$debugScriptFileName"
    local deployAndRunScriptPath="$buildPath/$deployAndRunScriptFileName"
    local deployAndDebugScriptPath="$buildPath/$deployAndDebugScriptFileName"

    echo
    echo "Generating $deployScriptFileName at $deployScriptPath..."
    echo "#! /bin/bash"                                                  > $deployScriptPath
    echo "echo \"Clearing old apk data from previous installation...\"" >> $deployScriptPath
    echo "$clearApkDataCommand"                                         >> $deployScriptPath
    echo "$installApkCommand"                                           >> $deployScriptPath

    echo
    echo "Generating $runScriptFileName at $runScriptPath..."
    echo "#! /bin/bash"                                                  > $runScriptPath
    echo "$runApkCommand"                                               >> $runScriptPath

    echo
    echo "Generating $debugScriptFileName at $debugScriptPath..."
    echo "#! /bin/bash"                                                  > $debugScriptPath
    echo "$runDebuggerCommand"                                          >> $debugScriptPath

    echo
    echo "Generating $deployScriptFileName at $deployScriptPath..."
    echo "#! /bin/bash"                                                  > $deployScriptPath
    echo "echo \"Clearing old apk data from previous installation...\"" >> $deployScriptPath
    echo "$clearApkDataCommand"                                         >> $deployScriptPath
    echo "$installApkCommand"  

    echo
    echo "Generating $deployAndRunScriptFileName at $deployAndRunScriptPath..."
    echo "#! /bin/bash"                                                  > $deployAndRunScriptPath
    echo "echo \"Clearing old apk data from previous installation...\"" >> $deployAndRunScriptPath
    echo "$clearApkDataCommand"                                         >> $deployAndRunScriptPath
    echo "$installApkCommand"                                           >> $deployAndRunScriptPath
    echo "$runApkCommand"                                               >> $deployAndRunScriptPath

    echo
    echo "Generating $deployAndDebugScriptFileName at $deployAndDebugScriptPath..."
    echo "#! /bin/bash"                                                  > $deployAndDebugScriptPath
    echo "echo \"Clearing old apk data from previous installation...\"" >> $deployAndDebugScriptPath
    echo "$clearApkDataCommand"                                         >> $deployAndDebugScriptPath
    echo "$installApkCommand"                                           >> $deployAndDebugScriptPath
    echo "$runDebuggerCommand"                                          >> $deployAndDebugScriptPath

    #FK: Set execute flag
    chmod +x $deployScriptPath
    chmod +x $runScriptPath
    chmod +x $debugScriptPath
    chmod +x $deployAndRunScriptPath
    chmod +x $deployAndDebugScriptPath
}

function main()
{
    #FK: Options:
    #       h - show help
    #       s - skip verification of base prerequisites
    #       i - install & run after compilation
    #       c - skip compilation
    while getopts "hsic" opt; do
        case ${opt} in
            h )
                printHelpText
                justPrintHelp=1
                ;;
            s )
                skipVerification=1
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
    shift 1

    if [ $# -eq 0 ]; then
        #FK: DEFAULT: Build for all architectures
        architectures+=("arm64" "arm32" "x86" "x64")
    else
        architectures=("$@")
    fi

    if ! evaluateArchitectures; then
        echo_error "Some or all of the architectures \"$architectures\" are not valid CPU architectures, aborting build process."
        exit
    fi

    if [ $skipVerification -eq 1 ]; then
        echo "Skipping installation verification..."
    else
        checkPrerequisites
        checkAndroidSDKManagerInstallation
        checkAndroidSDKInstallation
        checkJDKInstallation
        checkBundleToolInstallation

        #FK: Keystore file *has* to be present
        if ! checkKeystoreFile; then
            exit
        fi
    fi

    echo "Building using build mode '$buildMode' for architecture '$architecture'"

    createBuildDirectories

    stdLibIncludeDir=$(getAndroidNDKIncludePath)

    if ! generateAndroidManifest; then
        echo
        echo_error "Android Manifest creation failed, aborting apk building process"
        exit
    fi

    generateMakeFiles

    if ! compileUsingMakeFiles; then
        echo
        echo_error "Compilation failed, aborting apk building process"
        exit
    fi

    if ! generateApk; then
        echo_error "Error generating apk"
        exit
    fi

    if ! generateDebugSymbols; then
        echo_warning "Error while generating debug symbols"
    fi

    if [ $buildMode == "release" ]; then
        if ! generateAppBundle; then
            echo_error "Error generating app bundle"
            exit
        fi
    fi

    finalApkPath="$(getBuildPath $buildMode $architecture)/$projectName.apk"
    debugSymbolPath="$(getBuildPath $buildMode $architecture)/debug_symbols.zip"
    generateDeployScripts $finalApkPath

    echo "=================="
    echo
    echo
    echo "apk path:"
    echo $finalApkPath
    echo 
    echo "debug symbols:"
    echo $debugSymbolPath

    if [ "$buildMode" == "release" ]; then
        echo 
        echo
        echo "app bundle path (aab):"
        echo "$(getBuildPath $buildMode $architecture)/$projectName.aab"
    fi
}

main $@
