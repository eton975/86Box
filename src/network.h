/*
 * 86Box	A hypervisor and IBM PC system emulator that specializes in
 *		running old operating systems and software designed for IBM
 *		PC systems and compatibles from 1981 through fairly recent
 *		system designs based on the PCI bus.
 *
 *		This file is part of the 86Box distribution.
 *
 *		Definitions for the network module.
 *
 * Version:	@(#)network.h	1.0.2	2017/05/11
 *
 * Authors:	Kotori, <oubattler@gmail.com>
 *		Fred N. van Kempen, <decwiz@yahoo.com>
 */
#ifndef NETWORK_H
# define NETWORK_H
# include <stdint.h>


#define NE1000		1
#define NE2000		2
#define RTL8029AS	3


typedef void (*NETRXCB)(void *, uint8_t *, int);


typedef struct {
    char	name[64];
    char	internal_name[32];
    device_t	*device;
    void	*private;
    int		(*poll)(void *);
    NETRXCB	rx;
} netcard_t;

typedef struct {
    char	device[128];
    char	description[128];
} netdev_t;


/* Global variables. */
extern int	network_card;
extern int	network_type;


/* Function prototypes. */
extern void	network_init(void);
extern void	network_setup(char *);
extern int	network_attach(void *, uint8_t *, NETRXCB);
extern void	network_close(void);
extern void	network_reset(void);
extern void	network_tx(uint8_t *, int);

extern int	network_pcap_setup(uint8_t *, NETRXCB, void *);
extern void	network_pcap_close(void);
extern void	network_pcap_in(uint8_t *, int);
extern int	network_devlist(netdev_t *);

extern int	network_slirp_setup(uint8_t *, NETRXCB, void *);
extern void	network_slirp_close(void);
extern void	network_slirp_in(uint8_t *, int);

extern int	network_devlist(netdev_t *);
extern int	network_card_available(int);
extern char	*network_card_getname(int);
extern int	network_card_has_config(int);
extern char	*network_card_get_internal_name(int);
extern int	network_card_get_from_internal_name(char *);
extern struct device_t *network_card_getdevice(int);


#endif	/*NETWORK_H*/
