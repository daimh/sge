add_library(Xbae STATIC
	Actions.c
	Caption.c
	Cell.c
	Clip.c
	Converters.c
	Draw.c
	Input.c
	MCreate.c
	Matrix.c
	Methods.c
	Public.c
	ScrollMgr.c
	Shadow.c
	Utils.c
)
target_include_directories(Xbae PRIVATE
	/usr/include/freetype2
	../../../clients/common
	../../../common
	../../../daemons/common
	../../../daemons/qmaster
	../../../libs
	../../../libs/cull
	../../../libs/sgeobj
	../../../libs/uti
	../Xmt310
)
