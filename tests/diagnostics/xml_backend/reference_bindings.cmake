# SPDX-License-Identifier: GPL-3.0-only

# These forms belong to the immutable, pre-rename PR #108 inputs. Bind only
# generated copies; never rename the pinned checkout or its vendor sources.
function(xml_bind_legacy_reference variable)
  set(_xml_bound "${${variable}}")
  string(REPLACE "#include \"preloader_l/" "#include \"citrine_eclipse_mirage/"
    _xml_bound "${_xml_bound}")
  string(REPLACE "preloader_l::" "citrine_eclipse_mirage::"
    _xml_bound "${_xml_bound}")
  string(REPLACE "using namespace preloader_l;" "using namespace citrine_eclipse_mirage;"
    _xml_bound "${_xml_bound}")
  set(${variable} "${_xml_bound}" PARENT_SCOPE)
endfunction()

function(xml_read_reference_header reference_root variable)
  # This path is fixed by the reviewed historical commit, not today's identity.
  file(READ "${reference_root}/include/preloader_l/xml/reader.hpp" _xml_header)
  xml_bind_legacy_reference(_xml_header)
  set(${variable} "${_xml_header}" PARENT_SCOPE)
endfunction()
