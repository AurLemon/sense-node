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
		document.documentElement.classList.toggle('dark', themeMode === 'dark')
		document.documentElement.classList.toggle('light', themeMode === 'light')
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
	<UApp>
		<MiniHud v-if="isHud" />
		<main
			v-else
			class="grid h-screen w-screen grid-rows-[auto_minmax(0,1fr)_auto] overflow-hidden bg-muted text-default"
		>
			<header
				class="flex items-center justify-between gap-4 border-b border-default px-6 py-4"
			>
				<div class="min-w-0">
					<div class="text-[18px] font-semibold leading-none">
						{{ t('app.title') }}
					</div>
					<div
						class="mt-1 font-['JetBrains_Mono','MiSans',monospace] text-xs text-muted"
					>
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
				<div class="flex items-center gap-2">
					<USelect
						v-model="settings.locale"
						:items="localeItems"
						:placeholder="t('common.language')"
					/>
					<USelect
						v-model="settings.themeMode"
						:items="themeItems"
						:placeholder="t('common.theme')"
					/>
					<UBadge
						color="neutral"
						variant="soft"
						class="font-['JetBrains_Mono','MiSans',monospace] text-xs"
					>
						{{ t('app.rawStable') }} {{ device.rawEvent }} ·
						{{ t('app.stable') }}
						{{ device.stableEvent }}
					</UBadge>
				</div>
			</header>

			<section class="min-h-0 overflow-hidden p-3.5">
				<component :is="tabComponent" />
			</section>

			<BottomTabs v-model="activeTab" />
		</main>
	</UApp>
</template>
