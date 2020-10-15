/* GNU/(Solaris?) linker script to export only symbols matching "drmaa_*",
   hiding the general SGE symbols. */
{
  global: drmaa_*;
  	  Java_com_sun_grid_*;
  local: *;
};
