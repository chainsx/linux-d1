// SPDX-License-Identifier: GPL-2.0-only
/*
 * RISC-V specific functions to support DMA for non-coherent devices
 *
 * Copyright (c) 2021 Western Digital Corporation or its affiliates.
 */

#include <linux/dma-direct.h>
#include <linux/dma-iommu.h>
#include <linux/dma-map-ops.h>
#include <linux/mm.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <asm/cacheflush.h>

static unsigned int riscv_cbom_block_size = L1_CACHE_BYTES;
static bool noncoherent_supported;

void arch_sync_dma_for_device(phys_addr_t paddr, size_t size,
			      enum dma_data_direction dir)
{
	void *vaddr = phys_to_virt(paddr);

	switch (dir) {
	case DMA_TO_DEVICE:
		ALT_CMO_OP(clean, vaddr, size, riscv_cbom_block_size);
		break;
	case DMA_FROM_DEVICE:
		ALT_CMO_OP(clean, vaddr, size, riscv_cbom_block_size);
		break;
	case DMA_BIDIRECTIONAL:
		ALT_CMO_OP(flush, vaddr, size, riscv_cbom_block_size);
		break;
	default:
		break;
	}
}

void arch_sync_dma_for_cpu(phys_addr_t paddr, size_t size,
			   enum dma_data_direction dir)
{
	void *vaddr = phys_to_virt(paddr);

	switch (dir) {
	case DMA_TO_DEVICE:
		break;
	case DMA_FROM_DEVICE:
	case DMA_BIDIRECTIONAL:
		ALT_CMO_OP(flush, vaddr, size, riscv_cbom_block_size);
		break;
	default:
		break;
	}
}

void arch_dma_prep_coherent(struct page *page, size_t size)
{
	void *flush_addr = page_address(page);

	ALT_CMO_OP(flush, flush_addr, size, riscv_cbom_block_size);
}

void arch_setup_dma_ops(struct device *dev, u64 dma_base, u64 size,
		const struct iommu_ops *iommu, bool coherent)
{
	WARN_TAINT(!coherent && riscv_cbom_block_size > ARCH_DMA_MINALIGN,
		   TAINT_CPU_OUT_OF_SPEC,
		   "%s %s: ARCH_DMA_MINALIGN smaller than riscv,cbom-block-size (%d < %d)",
		   dev_driver_string(dev), dev_name(dev),
		   ARCH_DMA_MINALIGN, riscv_cbom_block_size);

	WARN_TAINT(!coherent && !noncoherent_supported, TAINT_CPU_OUT_OF_SPEC,
		   "%s %s: device non-coherent but no non-coherent operations supported",
		   dev_driver_string(dev), dev_name(dev));

	dev->dma_coherent = coherent;

	if (iommu)
		iommu_setup_dma_ops(dev, dma_base, dma_base + size - 1);
}

#ifdef CONFIG_RISCV_ISA_ZICBOM
void riscv_init_cbom_blocksize(void)
{
	struct device_node *node;
	int ret;
	u32 val;

	for_each_of_cpu_node(node) {
		int hartid = riscv_of_processor_hartid(node);
		int cbom_hartid;

		if (hartid < 0)
			continue;

		/* set block-size for cbom extension if available */
		ret = of_property_read_u32(node, "riscv,cbom-block-size", &val);
		if (ret)
			continue;

		if (!riscv_cbom_block_size) {
			riscv_cbom_block_size = val;
			cbom_hartid = hartid;
		} else {
			if (riscv_cbom_block_size != val)
				pr_warn("cbom-block-size mismatched between harts %d and %d\n",
					cbom_hartid, hartid);
		}
	}
}
#endif

void riscv_noncoherent_supported(void)
{
	noncoherent_supported = true;
}
