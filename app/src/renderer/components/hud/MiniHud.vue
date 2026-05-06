<script setup lang="ts">
import { computed } from 'vue'
import { useI18n } from '../../lib/i18n'
import { useDeviceStore } from '../../stores/deviceStore'
import { useSerialStore } from '../../stores/serialStore'

const device = useDeviceStore()
const serial = useSerialStore()
const { t } = useI18n()

const statusLabel = computed(() =>
	device.currentFrame
		? t(`serial.status.${serial.status.state}`)
		: t('device.notConnected'),
)

const hint = computed(() => {
	if (!device.currentFrame) return t('device.notConnected')
	if (device.stableEvent === 'tap') return t('hud.tapDetected')
	if (device.stableEvent === 'hand_near') return t('hud.handNear')
	if (device.stableEvent === 'hand_hover') return t('hud.handHover')
	return ''
})
</script>

<template>
	<button
		class="hud"
		type="button"
		@click="window.sensenode.app.showMainWindow()"
	>
		<div class="hud-head">
			<span>{{ t('app.title') }}</span>
			<span class="mono">{{ statusLabel }}</span>
		</div>
		<div class="mono muted">
			{{ t('hud.event') }}:
			{{ device.currentFrame ? device.stableEvent : '-' }} ·
			{{ t('scene.tofMm') }}: {{ device.currentFrame?.tof_mm ?? '-' }}mm
		</div>
		<div class="mono hint">
			{{
				hint ||
				`${t('hud.confidence')} ${device.currentFrame?.confidence?.toFixed(2) ?? '-'}`
			}}
		</div>
	</button>
</template>

<style scoped>
.hud {
	width: 100vw;
	height: 100vh;
	border: 1px solid var(--color-border);
	border-radius: 8px;
	padding: 13px 14px;
	background: rgb(8 10 15 / 78%);
	color: var(--color-text);
	backdrop-filter: blur(20px);
	text-align: left;
	-webkit-app-region: drag;
}

.hud-head {
	display: flex;
	align-items: center;
	justify-content: space-between;
	margin-bottom: 8px;
	font-size: 13px;
}

.hint {
	margin-top: 7px;
	color: var(--color-text-strong);
	font-size: 12px;
}
</style>
