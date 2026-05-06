<script setup lang="ts">
import { computed, onBeforeUnmount, onMounted, ref, watch } from 'vue'
import BottomTabs from './components/layout/BottomTabs.vue'
import MiniHud from './components/hud/MiniHud.vue'
import SenseNodeScene from './components/three/SenseNodeScene.vue'
import DeviceStatusPanel from './components/panels/DeviceStatusPanel.vue'
import EventTimelinePanel from './components/panels/EventTimelinePanel.vue'
import SerialMonitorPanel from './components/panels/SerialMonitorPanel.vue'
import LlmPanel from './components/panels/LlmPanel.vue'
import SettingsPanel from './components/panels/SettingsPanel.vue'
import DropdownSelect from './components/ui/DropdownSelect.vue'
import { useI18n } from './lib/i18n'
import { useDeviceStore } from './stores/deviceStore'
import { useSerialStore } from './stores/serialStore'
import { useSettingsStore } from './stores/settingsStore'

const device = useDeviceStore()
const serial = useSerialStore()
const settings = useSettingsStore()
const { t } = useI18n()
const activeTab = ref('scene')
const isHud = window.location.hash === '#hud'
let cleanupListeners: Array<() => void> = []

const tabComponent = computed(() => {
	if (activeTab.value === 'device') return DeviceStatusPanel
	if (activeTab.value === 'events') return EventTimelinePanel
	if (activeTab.value === 'serial') return SerialMonitorPanel
	if (activeTab.value === 'ai') return LlmPanel
	if (activeTab.value === 'settings') return SettingsPanel
	return SenseNodeScene
})

const localeItems = computed(() => [
	{ value: 'zh-CN', label: t('locale.zh') },
	{ value: 'en-US', label: t('locale.en') },
])

const themeItems = computed(() => [
	{ value: 'dark', label: t('theme.dark') },
	{ value: 'light', label: t('theme.light') },
])

watch(
	() => settings.eventStabilizerEnabled,
	(enabled) => {
		device.stabilizerEnabled = enabled
	},
	{ immediate: true },
)

watch(
	() => settings.locale,
	(locale) => {
		document.documentElement.lang = locale
	},
	{ immediate: true },
)

watch(
	() => settings.themeMode,
	(themeMode) => {
		document.documentElement.dataset.theme = themeMode
		document.documentElement.style.colorScheme = themeMode
	},
	{ immediate: true },
)

onMounted(() => {
	void serial.refreshPorts()
	cleanupListeners = [
		window.sensenode.device.onFrame((frame) =>
			device.ingestFrame(frame, 'serial'),
		),
		window.sensenode.device.onStatus((status) => {
			serial.status = status
		}),
		window.sensenode.serial.onLine(serial.pushLine),
	]
})

onBeforeUnmount(() => {
	for (const cleanup of cleanupListeners) cleanup()
	cleanupListeners = []
})
</script>

<template>
	<MiniHud v-if="isHud" />
	<main v-else class="app-shell">
		<header class="topbar">
			<div>
				<div class="brand">{{ t('app.title') }}</div>
				<div class="subline mono">
					{{
						device.currentFrame
							? t(`serial.status.${serial.status.state}`)
							: t('device.notConnected')
					}}
					/ {{ device.source ?? '-' }} / {{ t('app.fps') }}
					{{ serial.status.fps }} / {{ t('app.inference') }}
					{{ device.currentFrame?.inference_ms ?? '-' }}ms
				</div>
			</div>
			<div class="topbar-actions">
				<DropdownSelect
					v-model="settings.locale"
					class="topbar-select"
					:items="localeItems"
					:label="t('common.language')"
				/>
				<DropdownSelect
					v-model="settings.themeMode"
					class="topbar-select"
					:items="themeItems"
					:label="t('common.theme')"
				/>
				<div class="status-pill mono">
					{{ t('app.rawStable') }} {{ device.rawEvent }} · {{ t('app.stable') }}
					{{ device.stableEvent }}
				</div>
			</div>
		</header>

		<section class="content">
			<component :is="tabComponent" />
		</section>

		<BottomTabs v-model="activeTab" />
	</main>
</template>

<style scoped>
.app-shell {
	display: grid;
	grid-template-rows: auto 1fr auto;
	width: 100%;
	height: 100%;
	overflow: hidden;
	background:
		linear-gradient(180deg, rgb(255 255 255 / 3%), transparent 28%),
		var(--color-bg);
}

.topbar {
	display: flex;
	align-items: center;
	justify-content: space-between;
	padding: 18px 22px 14px;
	border-bottom: 1px solid var(--color-border);
	gap: 16px;
}

.brand {
	font-size: 18px;
	font-weight: 650;
	letter-spacing: 0;
}

.subline {
	margin-top: 5px;
	color: var(--color-text-subtle);
	font-size: 12px;
}

.topbar-actions {
	display: flex;
	align-items: center;
	gap: 10px;
}

.topbar-select {
	min-width: 122px;
}

.status-pill {
	border: 1px solid var(--color-border);
	border-radius: 999px;
	padding: 7px 10px;
	background: var(--color-surface-soft);
	color: var(--color-text-strong);
	font-size: 12px;
}

.content {
	min-height: 0;
	padding: 14px;
	overflow: hidden;
}
</style>
