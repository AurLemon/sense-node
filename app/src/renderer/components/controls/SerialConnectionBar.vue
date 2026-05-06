<script setup lang="ts">
import { computed } from 'vue'
import DropdownSelect from '../ui/DropdownSelect.vue'
import { getTransportLabel, useI18n } from '../../lib/i18n'
import { useSerialStore } from '../../stores/serialStore'

const serial = useSerialStore()
const { t, locale } = useI18n()

const portItems = computed(() =>
	serial.ports.map((port) => {
		const transportLabel = getTransportLabel(locale.value, port.transport)
		const parts = [port.manufacturer, port.friendlyName].filter(Boolean)

		return {
			value: port.path,
			label: port.path,
			description: parts.length > 0 ? parts.join(' · ') : t('serial.autoPick'),
			badge: port.isLikelyDevice
				? `${transportLabel} · ${t('serial.likely')}`
				: transportLabel,
		}
	}),
)

const stateLabel = computed(() => t(`serial.status.${serial.status.state}`))
const selectedLabel = computed(
	() =>
		serial.ports.find((port) => port.path === serial.selectedPort)?.path ??
		serial.selectedPort ??
		t('common.none'),
)

async function refresh(): Promise<void> {
	await serial.refreshPorts()
}

async function connect(): Promise<void> {
	await serial.connect()
}

async function disconnect(): Promise<void> {
	await serial.disconnect()
}
</script>

<template>
	<div class="panel controls">
		<div class="selector">
			<div class="label-row">
				<label>{{ t('serial.device') }}</label>
				<span class="mono meta">{{ stateLabel }}</span>
			</div>
			<DropdownSelect
				v-model="serial.selectedPort"
				:items="portItems"
				:placeholder="t('serial.selectPort')"
			/>
			<div class="hint mono">
				{{ t('serial.hint') }} · {{ t('common.selected') }}: {{ selectedLabel }}
			</div>
		</div>

		<label class="baud">
			<span>{{ t('serial.baudRate') }}</span>
			<input
				v-model.number="serial.baudRate"
				class="input"
				type="number"
				min="9600"
			/>
		</label>

		<div class="actions">
			<button class="button" type="button" @click="refresh">
				{{ t('common.refresh') }}
			</button>
			<button class="button" type="button" @click="connect">
				{{ t('common.connect') }}
			</button>
			<button class="button" type="button" @click="disconnect">
				{{ t('common.disconnect') }}
			</button>
		</div>
	</div>
</template>

<style scoped>
.controls {
	display: grid;
	grid-template-columns: minmax(0, 1fr) 140px auto;
	gap: 10px;
	padding: 12px;
}

.selector {
	display: grid;
	gap: 8px;
	min-width: 0;
}

.label-row {
	display: flex;
	align-items: center;
	justify-content: space-between;
	gap: 10px;
}

label {
	color: var(--color-text-muted);
	font-size: 12px;
}

.meta,
.hint {
	color: var(--color-text-subtle);
	font-size: 12px;
}

.hint {
	overflow: hidden;
	text-overflow: ellipsis;
	white-space: nowrap;
}

.baud {
	display: grid;
	align-content: start;
	gap: 8px;
}

.actions {
	display: grid;
	grid-auto-flow: column;
	align-content: start;
	gap: 8px;
}
</style>
