%global _duplicate_files_terminate_build 0
%global commit 66374180faffcc72a9a8634fd106b4ab19ce7333
%global shortcommit %(c=%{commit}; echo ${c:0:7})
%global commitdate 20260422
%global min_version 3.49.0

Name:     OpenVSP
Version:  3.49.0^%{commitdate}git%{shortcommit}
Release:  %autorelease
Summary:  A parametric aircraft geometry tool
License:  NASA-1.3
URL:      https://www.openvsp.org
Source:   https://github.com/Mumbri-systems/OpenVSP/archive/%{commit}.tar.gz
BuildRequires: libxml2-devel >= 2.12.10, gcc-c++ >= 14.2.1-2, openjpeg-devel >= 2.5.4-1, glm-devel >= 1.0.1, cminpack-devel >= 1.3.8, glew-devel >= 2.2.0, swig >= 4.3.0, doxygen >= 1.14.0-5, graphviz >= 12, texlive-scheme-basic, python3-devel >= 3.13, conda >= 24
BuildRequires:  cmake >= 3.31, gcc, gcc-c++, rpm-build >= 4
# BuildRequires: eigen3-devel major version change to 5.x breaks OpenABF.
#fltk-fluid, fltk-devel,
BuildRequires: angelscript
BuildRequires: angelscript-devel
BuildRequires: mesa-libGL-devel, mesa-libGLU-devel
BuildRequires: python3-numpy

%description
OpenVSP is a parametric aircraft geometry tool. OpenVSP allows the user to
create a 3D model of an aircraft defined by common engineering parameters.
This model can be processed into formats suitable for engineering analysis.

%prep
%autosetup -n OpenVSP-%{commit}

%build
# build third-party libraries
pushd Libraries
%cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DVSP_USE_SYSTEM_LIBXML2=true \
    -DVSP_USE_SYSTEM_FLTK=false \
    -DVSP_USE_SYSTEM_GLM=true \
    -DVSP_USE_SYSTEM_GLEW=true \
    -DVSP_USE_SYSTEM_CMINPACK=true \
    -DVSP_USE_SYSTEM_EIGEN=false \
    -DVSP_USE_SYSTEM_ANGELSCRIPT=true \
    -DVSP_USE_SYSTEM_CPPTEST=false
%cmake_build
popd

# build main
pushd src
%cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DVSP_LIBRARY_PATH=%{_builddir}/OpenVSP-%{commit}/Libraries/redhat-linux-build
%cmake_build

%install
pushd src
%cmake_install
install -Dm 644 %{_builddir}/OpenVSP-%{commit}/pkg/rpm/openvsp.desktop %{buildroot}%{_datadir}/applications/openvsp.desktop
install -Dm 644 %{_builddir}/OpenVSP-%{commit}/vspIcon.png %{buildroot}%{_datadir}/icons/hicolor/scalable/apps/openvsp.png
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
