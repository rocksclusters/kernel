/*
   libata-dump.c - helper library for SATA diskdump
*/

#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/spinlock.h>
#include <scsi/scsi.h>
#include "scsi.h"
#include <scsi/scsi_host.h>
#include <linux/libata.h>
#include <asm/uaccess.h>

#include "libata.h"

int ata_scsi_dump_sanity_check(struct scsi_device *sdev)
{
	struct ata_port *ap;
	struct ata_device *dev;

	ap = (struct ata_port *) &sdev->host->hostdata[0];
	dev = ata_scsi_find_dev(ap, sdev);

	if (!ata_dev_present(dev))
		return -EIO;
	if (ap->flags & ATA_FLAG_PORT_DISABLED)
		return -EIO;

	return 0;
}

static int ata_scsi_dump_run_bottomhalf(struct ata_port *ap)
{
	static struct pt_regs regs;	/* dummy */
	struct ata_host_set *host_set;
	struct ata_queued_cmd *qc;
	int handled = 0;

	host_set = ap->host_set;

	if (!list_empty(&ap->pio_task.entry)) {
		list_del_init(&ap->pio_task.entry);
		clear_bit(0, &ap->pio_task.pending);

		ata_pio_task(ap);
		handled = 1;
	}

	qc = ata_qc_from_tag(ap, ap->active_tag);
	if (qc) {
		ap->ops->irq_handler(host_set->irq, host_set, &regs);
		handled = 1;
	}

	return handled;
}

int ata_scsi_dump_quiesce(struct scsi_device *sdev)
{
	struct ata_port *ap;
	struct ata_device *dev;
	int handled;

	ap = (struct ata_port *) &sdev->host->hostdata[0];
	dev = ata_scsi_find_dev(ap, sdev);

	do {
		handled = ata_scsi_dump_run_bottomhalf(ap);
	} while (handled);

	if (ap->flags & ATA_FLAG_PORT_DISABLED)
		return -EIO;

	return 0;
}

void ata_scsi_dump_poll(struct scsi_device *sdev)
{
	struct ata_port *ap;
	struct ata_device *dev;

	ap = (struct ata_port *) &sdev->host->hostdata[0];
	dev = ata_scsi_find_dev(ap, sdev);

	if (ap->flags & ATA_FLAG_PORT_DISABLED) {
		printk(KERN_ERR "ata%u(%u): port disabled\n",
		       ap->id, dev->devno);
		return;
	}

	ata_scsi_dump_run_bottomhalf(ap);
}
