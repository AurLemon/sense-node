<script setup lang="ts">
defineProps<{
	items: Array<{ label: string; value: string | number | null | undefined }>
}>()
</script>

<template>
	<div class="metrics">
		<div v-for="item in items" :key="item.label" class="metric panel">
			<div class="label">{{ item.label }}</div>
			<div class="value mono">{{ item.value ?? '-' }}</div>
		</div>
	</div>
</template>

<style scoped>
.metrics {
	display: grid;
	grid-template-columns: repeat(4, minmax(0, 1fr));
	gap: 10px;
}

.metric {
	padding: 10px;
	min-height: 64px;
}

.label {
	color: var(--color-text-subtle);
	font-size: 12px;
}

.value {
	margin-top: 8px;
	font-size: 16px;
	color: var(--color-text);
	overflow: hidden;
	text-overflow: ellipsis;
	white-space: nowrap;
}
</style>
