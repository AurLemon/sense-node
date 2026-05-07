<script setup lang="ts">
import { computed } from 'vue'
import AppIcon from '../ui/AppIcon.vue'

const props = withDefaults(
	defineProps<{
		items: Array<{
			label: string
			value: string | number | null | undefined
			icon?: string
		}>
		columns?: 1 | 2 | 3 | 4
	}>(),
	{
		columns: 4,
	},
)

const gridClassName = computed(() => {
	const columnClassMap = {
		1: 'grid-cols-1',
		2: 'grid-cols-2',
		3: 'grid-cols-3',
		4: 'grid-cols-4',
	} as const

	return ['grid', columnClassMap[props.columns], 'gap-2.5'].join(' ')
})
</script>

<template>
	<div :class="gridClassName">
		<div
			v-for="item in items"
			:key="item.label"
			class="min-h-16 rounded-lg border border-default bg-default/75 p-2.5 backdrop-blur"
		>
			<div class="flex items-center gap-1.5 text-xs text-muted">
				<AppIcon
					v-if="item.icon"
					:name="item.icon"
					:size="14"
					class="shrink-0 text-muted"
				/>
				<span>{{ item.label }}</span>
			</div>
			<div
				class="mt-2 overflow-hidden text-ellipsis whitespace-nowrap text-base text-default"
			>
				{{ item.value ?? '-' }}
			</div>
		</div>
	</div>
</template>
