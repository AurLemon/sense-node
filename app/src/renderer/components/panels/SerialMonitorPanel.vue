<script setup lang="ts">
import SerialConnectionBar from '../controls/SerialConnectionBar.vue'
import { useI18n } from '../../lib/i18n'
import { useSerialStore } from '../../stores/serialStore'

const serial = useSerialStore()
const { t } = useI18n()
</script>

<template>
	<div class="serial-page">
		<SerialConnectionBar />

		<div class="panel lines mono">
			<div class="status-line">
				{{ t(`serial.status.${serial.status.state}`) }} ·
				{{ serial.status.selectedPort ?? serial.selectedPort ?? '-' }}
			</div>
			<div
				v-for="line in serial.lines"
				:key="line.id"
				:class="['line', line.level]"
			>
				<span>{{ new Date(line.timestamp).toLocaleTimeString() }}</span>
				<span>{{ line.message ?? line.line }}</span>
				<code v-if="line.message">{{ line.line }}</code>
			</div>
		</div>
	</div>
</template>

<style scoped>
.serial-page {
	display: grid;
	grid-template-rows: auto 1fr;
	gap: 12px;
	height: 100%;
	min-height: 0;
}

.lines {
	overflow: auto;
	padding: 10px;
	font-size: 12px;
}

.status-line {
	position: sticky;
	top: 0;
	z-index: 1;
	padding: 6px 0 10px;
	border-bottom: 1px solid var(--color-border);
	color: var(--color-text-subtle);
	background: var(--color-bg);
}

.line {
	display: grid;
	grid-template-columns: 90px 1fr;
	gap: 12px;
	border-bottom: 1px solid var(--color-border);
	padding: 7px 0;
	color: var(--color-text-muted);
}

.line.warning {
	color: var(--color-warning);
}

.line.error {
	color: var(--color-danger);
}

code {
	grid-column: 2;
	color: var(--color-text-subtle);
	white-space: pre-wrap;
}
</style>
