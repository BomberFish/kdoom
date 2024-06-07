################################################################
#
# $Id:$
#
# $Log:$
#


ifeq ($(V),1)
	VB=''
else
	VB=@
endif

OBJS+=$(OBJDIR)/i_video_fbink.o
OBJS+=$(OBJDIR)/i_input_evdev.o
OBJS+=$(OBJDIR)/kill.o

CC=arm-kindlepw2-linux-gnueabi-gcc
CFLAGS+=-ggdb3 -O2 -I./FBInk -fPIC -Wunused-const-variable=0 -Wall
LDFLAGS+=-Wl,--gc-sections -L./FBInk/Release/static -L./FBInk/libevdev-staged/lib
CFLAGS+=-ggdb3 -Wall -DNORMALUNIX -DLINUX
LIBS+=-lm -lc -lfbink -l:libevdev.a

# ifneq ($(NOSDL),1)
# 	LIBS+= -lSDL
# endif

# subdirectory for objects
OBJDIR=build
OUTPUT=kdoom

SRC_DOOM = i_main.o i_sound.o dummy.o am_map.o doomdef.o doomstat.o dstrings.o d_event.o d_items.o d_iwad.o d_loop.o d_main.o d_mode.o d_net.o f_finale.o f_wipe.o g_game.o hu_lib.o hu_stuff.o info.o i_cdmus.o i_endoom.o i_joystick.o i_scale.o i_system.o i_timer.o memio.o m_argv.o m_bbox.o m_cheat.o m_config.o m_controls.o m_fixed.o m_menu.o m_misc.o m_random.o p_ceilng.o p_doors.o p_enemy.o p_floor.o p_inter.o p_lights.o p_map.o p_maputl.o p_mobj.o p_plats.o p_pspr.o p_saveg.o p_setup.o p_sight.o p_spec.o p_switch.o p_telept.o p_tick.o p_user.o r_bsp.o r_data.o r_draw.o r_main.o r_plane.o r_segs.o r_sky.o r_things.o sha1.o sounds.o statdump.o st_lib.o st_stuff.o s_sound.o tables.o v_video.o wi_stuff.o w_checksum.o w_file.o w_file_stdc_unbuffered.o w_main.o w_wad.o z_zone.o
OBJS += $(addprefix $(OBJDIR)/, $(SRC_DOOM))

all: $(OUTPUT)

clean: fbink_clean
	rm -rf $(OBJDIR)
	rm -f $(OUTPUT)
	rm -f $(OUTPUT).gdb
	rm -f $(OUTPUT).map

fbink_clean:
	$(MAKE) -C FBInk clean
	$(MAKE) -C FBInk libevdevclean

fbink: fbink_libevdev
	$(MAKE) -C FBInk static KINDLE=1 CROSS_COMPILE=arm-kindlepw2-linux-gnueabi-

fbink_libevdev:
	$(MAKE) -C FBInk libevdev.built KINDLE=1 CROSS_COMPILE=arm-kindlepw2-linux-gnueabi- CROSS_TC=arm-kindlepw2-linux-gnueabi CC=arm-kindlepw2-linux-gnueabi-gcc AR=arm-kindlepw2-linux-gnueabi-ar LD=arm-kindlepw2-linux-gnueabi-ld

package: $(OUTPUT)
	cp $(OUTPUT) ./kual/doom

dev: package
	rm -rf /run/media/$(shell whoami)/Kindle/extensions/doom
	cp -r ./kual/doom /run/media/$(shell whoami)/Kindle/extensions/

dev-ssh: package
	if [ ! -f .passwd ]; then echo "Please create a file named .passwd with the root password of your kindle"; exit 1; fi
	sshpass -p $(shell cat .passwd) ssh root@192.168.15.244 "uname -a"
	sshpass -p $(shell cat .passwd) ssh root@192.168.15.244 "rm -rf /mnt/us/extensions/doom"
	sshpass -p $(shell cat .passwd) scp -r ./kual/doom root@192.168.15.244:/mnt/us/extensions/


# copy-libs:
# 	if [ ! -f .passwd ]; then echo "Please create a file named .passwd with the root password of your kindle"; exit 1; fi
# 	if [ ! -d lib ]; then sshpass -p $(shell cat .passwd) scp root@192.168.15.244:/usr/lib .; else echo "libs already copied :)"; fi

run-ssh: dev-ssh
	if [ ! -f .passwd ]; then echo "Please create a file named .passwd with the root password of your kindle"; exit 1; fi
	sshpass -p $(shell cat .passwd) ssh root@192.168.15.244 "/mnt/us/extensions/doom/kdoom"

$(OUTPUT):	$(OBJS) fbink
	@echo [Linking $@]
	$(VB)$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) \
	-o $(OUTPUT) $(LIBS) -Wl,-Map,$(OUTPUT).map

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o:	src/%.c
	@echo [Compiling $<]
	$(VB)$(CC) $(CFLAGS) -c $< -o $@

print:
	@echo OBJS: $(OBJS)
