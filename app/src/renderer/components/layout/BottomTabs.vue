<script setup lang="ts">
import { computed } from 'vue'
import AppIcon from '../ui/AppIcon.vue'
import { useI18n } from '../../lib/i18n'

const model = defineModel<string>({ required: true })
const { t } = useI18n()

const tabs = computed(() => [
	{ id: 'scene', label: t('tab.scene'), icon: 'lucide:square-activity' },
	{ id: 'device', label: t('tab.device'), icon: 'lucide:cpu' },
	{ id: 'events', label: t('tab.events'), icon: 'lucide:bell-ring' },
	{ id: 'settings', label: t('tab.settings'), icon: 'lucide:settings' },
])
</script>

<template>
	<nav class="grid h-16 grid-cols-4 items-center gap-2 px-4">
		<UButton
			v-for="tab in tabs"
			:key="tab.id"
			class="h-10 w-full min-w-0 gap-2 justify-center"
			:color="model === tab.id ? 'primary' : 'neutral'"
			:variant="model === tab.id ? 'solid' : 'soft'"
			type="button"
			:title="tab.label"
			@click="model = tab.id"
		>
			<AppIcon :name="tab.icon" :size="16" />
			<span>{{ tab.label }}</span>
		</UButton>
	</nav>
</template>
