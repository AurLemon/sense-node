<script setup lang="ts">
import {
	Bot,
	Cpu,
	ListTree,
	Radio,
	Settings,
	SquareActivity,
} from 'lucide-vue-next'
import { computed } from 'vue'
import { useI18n } from '../../lib/i18n'

const model = defineModel<string>({ required: true })
const { t } = useI18n()

const tabs = computed(() => [
	{ id: 'scene', label: t('tab.scene'), icon: SquareActivity },
	{ id: 'device', label: t('tab.device'), icon: Cpu },
	{ id: 'events', label: t('tab.events'), icon: ListTree },
	{ id: 'serial', label: t('tab.serial'), icon: Radio },
	{ id: 'ai', label: t('tab.ai'), icon: Bot },
	{ id: 'settings', label: t('tab.settings'), icon: Settings },
])
</script>

<template>
	<nav class="tabs">
		<button
			v-for="tab in tabs"
			:key="tab.id"
			class="tab"
			:class="{ active: model === tab.id }"
			type="button"
			:title="tab.label"
			@click="model = tab.id"
		>
			<component :is="tab.icon" :size="16" />
			<span>{{ tab.label }}</span>
		</button>
	</nav>
</template>

<style scoped>
.tabs {
	display: grid;
	grid-template-columns: repeat(6, minmax(0, 1fr));
	gap: 8px;
	padding: 10px 14px 14px;
	border-top: 1px solid var(--color-border);
}

.tab {
	display: inline-flex;
	align-items: center;
	justify-content: center;
	gap: 7px;
	height: 40px;
	border: 1px solid var(--color-border);
	border-radius: 8px;
	background: var(--color-surface);
	color: var(--color-text-muted);
}

.tab.active {
	background: var(--color-surface-strong);
	color: var(--color-text);
}
</style>
