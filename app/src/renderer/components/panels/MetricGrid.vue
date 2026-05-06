<script setup lang="ts">
defineProps<{
	items: Array<{ label: string; value: string | number | null | undefined }>
}>()
</script>

<template>
	<div class="grid grid-cols-4 gap-2.5">
		<div
			v-for="item in items"
			:key="item.label"
			class="min-h-16 rounded-lg border border-default bg-default/75 p-2.5 shadow-sm backdrop-blur"
		>
			<div class="text-xs text-muted">{{ item.label }}</div>
			<div
				class="mt-2 overflow-hidden text-ellipsis whitespace-nowrap font-['JetBrains_Mono','MiSans',monospace] text-base text-default"
			>
				{{ item.value ?? '-' }}
			</div>
		</div>
	</div>
</template>
