<script setup lang="ts">
import { computed } from 'vue'
import AppIcon from '../ui/AppIcon.vue'
import { useI18n } from '../../lib/i18n'

const model = defineModel<string>({ required: true })
const { t } = useI18n()

const tabs = computed(() => [
	{ id: 'scene', label: t('tab.scene'), icon: 'lucide:square-activity' },
	{ id: 'device', label: t('tab.device'), icon: 'lucide:cpu' },
	{ id: 'events', label: t('tab.events'), icon: 'lucide:list-tree' },
	{ id: 'serial', label: t('tab.serial'), icon: 'lucide:radio' },
	{ id: 'ai', label: t('tab.ai'), icon: 'lucide:bot' },
	{ id: 'settings', label: t('tab.settings'), icon: 'lucide:settings' },
])
</script>

<template>
	<nav
		class="grid grid-cols-6 gap-2 border-t border-default px-3.5 pb-3.5 pt-2.5"
	>
		<UButton
			v-for="tab in tabs"
			:key="tab.id"
			class="h-10 w-full min-w-0 gap-2"
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
