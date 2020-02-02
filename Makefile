# Makefile for the TCAM Cache library 

CC      = gcc
CFLAGS  = -g
RM      = rm -f


default: all

all: tcam_entry_mgr

tcam_entry_mgr: tcam_mgr_main.c tcam_entry_mgr.c tcam.c
	$(CC) $(CFLAGS) -o tcam_entry_mgr tcam_mgr_main.c tcam_entry_mgr.c tcam.c

clean veryclean:
	$(RM) tcam_entry_mgr