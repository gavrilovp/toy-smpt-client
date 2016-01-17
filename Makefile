IDIR = include
CDIR = src
ODIR = obj

SMTPCLIENT = smtp-client

CC = gcc
CFLAGS = -I$(IDIR) -std=c99 -Wall -Werror
LDFLAGS = -lrt

all: $(SMTPCLIENT)

INCLUDES = $(wildcard $(IDIR)/*.h)

OBJS = $(addprefix $(ODIR)/, main.o toylog.o)

$(ODIR)/%.o: $(CDIR)/%.c $(INCLUDES)
	mkdir -p $(ODIR)
	$(CC) -c $(CFLAGS) -o $@ $<

$(SMTPCLIENT): $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^

.PHONY: clean
clean:
	rm -rf $(ODIR) $(CDIR)/*~ $(IDIR)/*~
	rm -f $(SMTPCLIENT)
	rm -f *.log
