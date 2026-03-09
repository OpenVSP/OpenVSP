%global _duplicate_files_terminate_build 0

Name:     OpenVSP
Version:  3.47.0
Release:  %autorelease
Summary:  A parametric aircraft geometry tool
License:  NASA-1.3
URL:      https://www.openvsp.org
Source:   https://github.com/Mumbri-systems/OpenVSP/archive/refs/heads/fedora-3.47.0.zip
BuildRequires: libxml2-devel >= 2.12.10-5, gcc-c++ >= 14.2.1-2, openjpeg-devel >= 2.5.4-1, glm-devel >= 1.0.1-5, cminpack-devel >= 1.3.8-10, glew-devel >= 2.2.0-11, swig >= 4.3.1-5, doxygen >= 1.14.0-5, graphviz >= 13.1.2-3, texlive-scheme-basic, python3-devel >= 3.14.3-1, conda >= 25.11.0-3
BuildRequires:  cmake >= 3.31.10-3, gcc14, gcc14-c++, rpm-build >= 6.0.1-1
#fltk-fluid, fltk-devel,
BuildRequires: mesa-libGL-devel, mesa-libGLU-devel
BuildRequires: python3-numpy

%description
OpenVSP is a parametric aircraft geometry tool. OpenVSP allows the user to
create a 3D model of an aircraft defined by common engineering parameters.
This model can be processed into formats suitable for engineering analysis.

%prep
%autosetup -n OpenVSP-OpenVSP_%{version}

%build
# build third-party libraries
mkdir buildlibs
pushd buildlibs
# set gcc-14 for libEigen
export CC=gcc-14
export CXX=g++-14

%cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DVSP_USE_SYSTEM_LIBXML2=true \
    -DVSP_USE_SYSTEM_FLTK=false \
    -DVSP_USE_SYSTEM_GLM=true \
    -DVSP_USE_SYSTEM_GLEW=true \
    -DVSP_USE_SYSTEM_CMINPACK=true \
    -DVSP_USE_SYSTEM_CPPTEST=false \
    -DCMAKE_C_COMPILER=gcc-14 \
    -DCMAKE_CXX_COMPILER=g++-14 \
    -S %{_builddir}/OpenVSP-OpenVSP_%{version}/Libraries \
    -B .
%make_build
popd

# build main
%cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DVSP_LIBRARY_PATH=%{_builddir}/OpenVSP-OpenVSP_%{version}/buildlibs \
    -S %{_builddir}/OpenVSP-OpenVSP_%{version}/src
%cmake_build

%install
%cmake_install
install -Dm 644 %{_builddir}/OpenVSP-OpenVSP_%{version}/pkg/rpm/openvsp.desktop %{buildroot}%{_datadir}/applications/openvsp.desktop
install -Dm 644 %{_builddir}/OpenVSP-OpenVSP_%{version}/vspIcon.png %{buildroot}%{_datadir}/icons/hicolor/scalable/apps/openvsp.png
# Remove git placeholder files
find %{buildroot} -name ".keep" -delete
chmod 755 %{buildroot}%{_prefix}/python/openvsp/conda-recipe/build.sh

%files
%{_bindir}/vsp
%{_bindir}/vspscript
%{_bindir}/vspaero
%{_bindir}/vspaero_opt
%{_bindir}/vspviewer
%{_bindir}/vsploads
%{_datadir}/openvsp/
%{_datadir}/applications/openvsp.desktop
%{_datadir}/icons/hicolor/scalable/apps/openvsp.png
%{_prefix}/help/
%{_prefix}/python/
%{_datadir}/doc/VSP/
%doc README.md
%license LICENSE

%changelog
%autochangelog
