<script setup lang="ts">
import { Icon } from '@iconify/vue'

withDefaults(
	defineProps<{
		name: string
		size?: number | string
		class?: string
	}>(),
	{
		size: 16,
		class: undefined,
	},
)
</script>

<template>
	<Icon :icon="name" :width="size" :height="size" :class="class" />
</template>
